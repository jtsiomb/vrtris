#include <stdio.h>
#include <stdlib.h>
#include "opengl.h"
#include "cmesh.h"


struct cmesh_vattrib {
	int nelem;	/* num elements per attribute [1, 4] */
	float *data;	/* dynarr */
	unsigned int vbo;
	int vbo_valid, data_valid;
};


struct cmesh {
	char *name;
	unsigned int nverts, nfaces;

	/* current value for each attribute for the immediate mode interface */
	cgm_vec4 cur_val[CMESH_NUM_ATTR];

	unsigned int buffer_objects[CMESH_NUM_ATTR + 1];
	struct cmesh_vattrib vattr[CMESH_NUM_ATTR];

	unsigned int *idata;	/* dynarr */
	unsigned int ibo;
	int ibo_valid, idata_valid;

	/* index buffer for wireframe rendering (constructed on demand) */
	unsigned int wire_ibo;
	int wire_ibo_valid;

	/* axis-aligned bounding box */
	cgm_vec3 aabb_min, aabb_max;
	int aabb_valid;
	/* bounding sphere */
	cgm_vec3 bsph_center;
	float bsph_radius;
	int bsph_valid;
};

static int sdr_loc[CMESH_NUM_ATTR] = {0, 1, 2, 3, 4, 5, 6, 7};


/* global state */
void cmesh_set_attrib_sdrloc(int attr, int loc)
{
	sdr_loc[attr] = loc;
}

int cmesh_get_attrib_sdrloc(int attr)
{
	return sdr_loc[attr];
}

void cmesh_clear_attrib_sdrloc(void)
{
	int i;
	for(i=0; i<CMESH_NUM_ATTR; i++) {
		sdr_loc[i] = -1;
	}
}

/* mesh functions */
struct cmesh *cmesh_alloc(void)
{
	struct cmesh *cm;

	if(!(cm = malloc(sizeof *cm))) {
		return 0;
	}
	if(cmesh_init(cm) == -1) {
		free(cm);
		return 0;
	}
	return cm;
}

void cmesh_free(struct cmesh *cm)
{
	cmesh_destroy(cm);
	free(cm);
}

int cmesh_init(struct cmesh *cm)
{
	int i;

	memset(cm, 0, sizeof *cm);
	cgm_wcons(cm->cur_val + CMESH_ATTR_COLOR, 1, 1, 1, 1);

	glGenBuffers(CMESH_NUM_ATTR + 1, cm->buffer_objects);

	for(i=0; i<CMESH_NUM_ATTR; i++) {
		if(!(cm->vattr[i].data = dynarr_alloc(0, sizeof(float)))) {
			cmesh_destroy(cm);
			return -1;
		}
		cm->vattr[i].vbo = buffer_objects[i];
	}

	cm->ibo = buffer_objects[CMESH_NUM_ATTR];
	if(!(cm->idata = dynarr_alloc(0, sizeof *cm->idata))) {
		cmesh_destroy(cm);
		return -1;
	}
	return 0;
}

void cmesh_destroy(struct cmesh *cm)
{
	int i;

	free(cm->name);

	for(i=0; i<CMESH_NUM_ATTR; i++) {
		dynarr_free(cm->vattr[i].data);
	}
	dynarr_free(cm->idata);

	glDeleteBuffers(CMESH_NUM_ATTR + 1, cm->buffer_objects);
	if(cm->wire_ibo) {
		glDeleteBuffers(1, &cm->wire_ibo);
	}
}

void cmesh_clear(struct cmesh *cm)
{
	int i;

	for(i=0; i<CMESH_NUM_ATTR; i++) {
		cm->vattr[i].nelem = 0;
		cm->vattr[i].vbo_valid = 0;
		cm->vattr[i].data_valid = 0;
		cm->vattr[i].data = dynarr_clear(cm->vattr[i].data);
	}
	cm->ibo_valid = cm->idata_valid = 0;
	cm->idata = dynarr_clear(cm->idata);

	cm->wire_ibo_valid = 0;
	cm->nverts = cm->nfaces = 0;

	cm->bsph_valid = cm->aabb_valid = 0;
}

int cmesh_clone(struct cmesh *cmdest, struct cmesh *cmsrc)
{
	int i, num, nelem;
	char *name = 0;
	float *varr[CMESH_NUM_ATTR] = {0};
	unsigned int *iarr = 0;

	/* do anything that can fail first, before making any changes to cmdest
	 * so we have the option of recovering gracefuly
	 */
	if(cmsrc->name) {
		if(!(name = malloc(strlen(cmsrc->name)))) {
			return -1;
		}
		strcpy(name, cmsrc->name);
	}
	if(cmesh_indexed(cmsrc)) {
		num = dynarr_size(cmsrc->idata);
		if(!(iarr = dynarr_alloc(num, sizeof *iarr))) {
			free(name);
			return -1;
		}
	}
	for(i=0; i<CMESH_NUM_ATTR; i++) {
		if(cmesh_has_attrib(cmsrc, i)) {
			nelem = cmsrc->vattr[i].nelem;
			num = dynarr_size(cmsrc->vattr[i].data);
			if(!(varr[i] = dynarr_alloc(num * nelem, sizeof(float)))) {
				while(--i >= 0) {
					dynarr_free(varr[i]);
				}
				dynarr_free(iarr);
				free(name);
				return -1;
			}
		}
	}

	cmesh_clear(cmdest);

	for(i=0; i<CMESH_NUM_ATTR; i++) {
		dynarr_free(cmdest->vattr[i].data);

		if(cmesh_has_attrib(cmsrc, i)) {
			cmesh_attrib(cmsrc, i);	/* force validation of the actual data on the source mesh */

			nelem = cmsrc->vattr[i].nelem;
			cmdest->vattr[i].nelem = nelem
			num = dynarr_size(cmsrc->vattr[i].data);
			cmdest->vattr[i].data = varr[i];
			memcpy(cmdest->vattr[i].data, cmsrc->vattr[i].data, num * nelem * sizeof(float));
			cmdest->vattr[i].data_valid = 1;
			cmdest->vattr[i].vbo_valid = 0;
		} else {
			memset(cmdest->vattr + i, 0, sizeof cmdest->vattr[i]);
		}
	}

	dynarr_free(cmdest->idata);
	if(cmesh_indexed(cmsrc)) {
		cmesh_index(cmsrc);	/* force validation .... */

		num = dynarr_size(cmsrc->idata);
		cmdest->idata = iarr;
		memcpy(cmdest->idata, cmsrc->idata, num * sizeof *cmdest->idata);
		cmdest->idata_valid = 1;
	} else {
		cmdest->idata = 0;
		cmdest->idata_valid = cmdest->ibo_valid = 0;
	}

	free(cmdest->name);
	cmdest->name = name;

	cmdest->nverts = cmsrc->nverts;
	cmdest->nfaces = cmsrc->nfaces;

	memcpy(cmdest->cur_val, cmsrc->cur_val, sizeof cmdest->cur_val);

	cmdest->aabb_min = cmsrc->aabb_min;
	cmdest->aabb_max = cmsrc->aabb_max;
	cmdest->aabb_valid = cmsrc->aabb_valid;
	cmdest->bsph_center = cmsrc->bsph_center;
	cmdest->bsph_radius = cmsrc->bsph_radius;
	cmdest->bsph_valid = cmsrc->bsph_valid;

	return 0;
}

int cmesh_set_name(struct cmesh *cm, const char *name)
{
	int len = strlen(name);
	char *tmp = malloc(len + 1);
	if(!tmp) return -1;
	free(cm->name);
	cm->name = tmp;
	memcpy(cm->name, name, len + 1);
	return 0;
}

const char *cmesh_name(struct cmesh *cm)
{
	return cm->name;
}

int cmesh_has_attrib(struct cmesh *cm, int attr)
{
	if(attr < 0 || attr >= CMESH_NUM_ATTR) {
		return 0;
	}
	return cm->vattr[attr].vbo_valid | cm->vattr[attr].data_valid;
}

int cmesh_indexed(struct cmesh *cm)
{
	return cm->ibo_valid | cm->idata_valid;
}

/* vdata can be 0, in which case only memory is allocated
 * returns pointer to the attribute array
 */
float *cmesh_set_attrib(struct cmesh *cm, int attr, int nelem, unsigned int num,
		const float *vdata)
{
	float *newarr;

	if(attr < 0 || attr >= CMESH_NUM_ATTR) {
		return 0;
	}
	if(cm->nverts && num != cm->nverts) {
		return 0;
	}

	if(!(newarr = dynarr_alloc(num * nelem, sizeof *newarr))) {
		return 0;
	}
	if(data) {
		memcpy(newarr, vdata, num * nelem * sizeof *newarr);
	}

	cm->nverts = num;

	dynarr_free(cm->vattr[attr].data);
	cm->vattr[attr].data = newarr;
	cm->vattr[attr].nelem = nelem;
	cm->vattr[attr].data_valid = 1;
	cm->vattr[attr].vbo_valid = 0;
	return newarr;
}

float *cmesh_attrib(struct cmesh *cm, int attr)
{
	if(attr < 0 || attr >= CMESH_NUM_ATTR) {
		return 0;
	}
	cm->vattr[attr].vbo_valid = 0;
	return (float*)cmesh_attrib_ro(cm, attr);
}

const float *cmesh_attrib_ro(struct cmesh *cm, int attr)
{
	float *tmp;
	int nelem;

	if(attr < 0 || attr >= CMESH_NUM_ATTR) {
		return 0;
	}

	if(!cm->vattr[attr].data_valid) {
#if GL_ES_VERSION_2_0
		return 0;
#else
		if(!cm->vattr[attr].vbo_valid) {
			return 0;
		}

		/* local data copy unavailable, grab the data from the vbo */
		nelem = cm->vattr[attr].nelem;
		if(!(tmp = dynarr_resize(cm->vattr[attr].data, cm->nverts * nelem))) {
			return 0;
		}
		cm->vattr[attr].data = tmp;

		glBindBuffer(GL_ARRAY_BUFFER, vattr[attr].vbo);
		tmp = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
		memcpy(cm->vattr[attr].data, tmp, cm->nverts * nelem * sizeof(float));
		glUnmapBuffer(GL_ARRAY_BUFFER);

		cm->vattr[attr].data_valid = 1;
#endif
	}
	return cm->vattr[attr].data;
}

int cmesh_attrib_count(struct cmesh *cm, int attr)
{
	return cmesh_has_attrib(cm, attr) ? cm->nverts : 0;
}

/* indices can be 0, in which case only memory is allocated
 * returns pointer to the index array
 */
unsigned int *cmesh_set_index(struct cmesh *cm, int num, const unsigned int *indices)
{
	unsigned int *tmp;
	int nidx = cm->nfaces * 3;

	if(nidx && num != nidx) {
		return 0;
	}

	if(!(tmp = dynarr_alloc(num, sizeof *tmp))) {
		return 0;
	}
	if(indices) {
		memcpy(tmp, indices, num * sizeof *tmp);
	}

	dynarr_free(cm->idata);
	cm->idata = tmp;
	cm->idata_valid = 1;
	cm->ibo_valid = 0;
	return tmp;
}

unsigned int *cmesh_index(struct cmesh *cm)
{
	cm->ibo_valid = 0;
	return (unsigned int*)cmesh_index_ro(cm);
}

const unsigned int *cmesh_index_ro(struct cmesh *cm)
{
	int nidx;
	unsigned int *tmp;

	if(!cm->idata_valid) {
#if GL_ES_VERSION_2_0
		return 0;
#else
		if(!cm->ibo_valid) {
			return 0;
		}

		/* local copy is unavailable, grab the data from the ibo */
		nidx = cm->nfaces * 3;
		if(!(tmp = dynarr_alloc(nidx, sizeof *cm->idata))) {
			return 0;
		}
		dynarr_free(cm->idata);
		cm->idata = tmp;

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cm->ibo);
		tmp = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);
		memcpy(cm->idata, tmp, nidx * sizeof *cm->idata);
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

		cm->idata_valid = 1;
	}
	return cm->idata;
}

int cmesh_index_count(struct cmesh *cm)
{
	return cm->nfaces * 3;
}

int get_poly_count(struct cmesh *cm)
{
	if(cm->nfaces) {
		return cm->nfaces;
	}
	if(cm->nverts) {
		return cm->nverts / 3;
	}
	return 0;
}

/* attr can be -1 to invalidate all attributes */
void cmesh_invalidate_vbo(struct cmesh *cm, int attr)
{
	int i;

	if(attr >= CMESH_NUM_ATTR) {
		return;
	}

	if(attr < 0) {
		for(i=0; i<CMESH_NUM_ATTR; i++) {
			cm->vattr[i].vbo_valid = 0;
		}
	} else {
		cm->vattr[attr].vbo_valid = 0;
	}
}

void cmesh_invalidate_index(struct cmesh *cm)
{
	cm->ibo_valid = 0;
}

int cmesh_append(struct cmesh *cmdest, struct cmesh *cmsrc)
{
	int i, nelem, nidx, newsz, origsz;
	float *vptr;
	unsigned int *iptr;
	unsigned int idxoffs;

	if(!cmdest->nverts) {
		return cmesh_clone(cmdest, cmsrc);
	}

	for(i=0; i<CMESH_NUM_ATTR; i++) {
		if(cmesh_has_attrib(cmdest) && cmesh_has_attrib(cmsrc)) {
			/* force validation of the data arrays */
			cmesh_attrib(cmdest, i);
			cmesh_attrib_ro(cmsrc, id);

			assert(cmdest->vattr[i].nelem == cmsrc->vattr[i].nelem);
			nelem = cmdest->vattr[i].nelem;
			origsz = cmdest->nverts * nelem;
			newsz = cmdest->nverts + cmsrc->nverts * nelem;

			if(!(vptr = dynarr_resize(cmdest->vattr[i].data, newsz))) {
				return -1;
			}
			memcpy(vptr + origsz, cmsrc->vattr[i].data, cmsrc->nverts * nelem * sizeof(float));
			cmdest->vattr[i].data = vptr;
		}
	}

	if(cmesh_indexed(cmdest)) {
		assert(cmesh_indexed(cmsrc));
		/* force validation ... */
		cmesh_index(cmdest);
		cmesh_index_ro(cmsrc);

		idxoff = cmdest->nverts;
		origsz = dynarr_size(cmdest->idata);
		srcsz = dynarr_size(cmsrc->idata);
		newsz = origsz + srcsz;

		if(!(iptr = dynarr_resize(cmdest->idata, newsz))) {
			return -1;
		}
		cmdest->idata = iptr;

		/* copy and fixup all the new indices */
		iptr += origsz;
		for(i=0; i<srcsz; i++) {
			*iptr++ = cmsrc->idata[i] + idxoffs;
		}
	}

	cmdest->wire_ibo_valid = 0;
	cmdest->aabb_valid = 0;
	cmdest->bsph_valid = 0;
}

/* assemble a complete vertex by adding all the useful attributes */
int cmesh_vertex(struct cmesh *cm, float x, float y, float z)
{
	int i, j;

	cgm_wcons(cm->cur_val + CMESH_ATTR_VERTEX, x, y, z, 1.0f);
	cm->vattr[CMESH_ATTR_VERTEX].data_valid = 1;
	cm->vattr[CMESH_ATTR_VERTEX].nelem = 3;

	for(i=0; i<CMESH_ATTR_VERTEX; i++) {
		if(cm->vattr[i].data_valid) {
			for(j=0; j<cm->vattr[CMESH_ATTR_VERTEX].nelem; j++) {
				float *tmp = dynarr_push(cm->vattr[i].data, cur_val[i] + j);
				if(!tmp) return -1;
				cm->vattr[i].data = tmp;
			}
		}
		cm->vattr[i].vbo_valid = 0;
		cm->vattr[i].data_valid = 1;
	}

	if(cm->idata_valid) {
		cm->idata = dynarr_clear(cm->idata);
	}
	cm->ibo_valid = cm->idata_valid = 0;
	return 0;
}

void cmesh_normal(struct cmesh *cm, float nx, float ny, float nz)
{
	cgm_wcons(cm->cur_val + CMESH_ATTR_NORMAL, nx, ny, nz, 1.0f);
	cm->vattr[CMESH_ATTR_NORMAL].nelem = 3;
}

void cmesh_tangent(struct cmesh *cm, float tx, float ty, float tz)
{
	cgm_wcons(cm->cur_val + CMESH_ATTR_TANGENT, tx, ty, tz, 1.0f);
	cm->vattr[CMESH_ATTR_TANGENT].nelem = 3;
}

void cmesh_texcoord(struct cmesh *cm, float u, float v, float w)
{
	cgm_wcons(cm->cur_val + CMESH_ATTR_TEXCOORD, u, v, w, 1.0f);
	cm->vattr[CMESH_ATTR_TEXCOORD].nelem = 3;
}

void cmesh_boneweights(struct cmesh *cm, float w1, float w2, float w3, float w4)
{
	cgm_wcons(cm->cur_val + CMESH_ATTR_BONEWEIGHTS, w1, w2, w3, w4);
	cm->vattr[CMESH_ATTR_BONEWEIGHTS].nelem = 4;
}

void cmesh_boneidx(struct cmesh *cm, int idx1, int idx2, int idx3, int idx4)
{
	cgm_wcons(cm->cur_val + CMESH_ATTR_BONEIDX, idx1, idx2, idx3, idx4);
	cm->vattr[CMESH_ATTR_BONEIDX].nelem = 4;
}

/* dir_xform can be null, in which case it's calculated from xform */
void cmesh_apply_xform(struct cmesh *cm, float *xform, float *dir_xform);

void cmesh_flip(struct cmesh *cm);	/* flip faces (winding) and normals */
void cmesh_flip_faces(struct cmesh *cm);
void cmesh_flip_normals(struct cmesh *cm);

void cmesh_explode(struct cmesh *cm);	/* undo all vertex sharing */

/* this is only guaranteed to work on an exploded mesh */
void cmesh_calc_face_normals(struct cmesh *cm);

void cmesh_draw(struct cmesh *cm);
void cmesh_draw_wire(struct cmesh *cm, float linesz);
void cmesh_draw_vertices(struct cmesh *cm, float ptsz);
void cmesh_draw_normals(struct cmesh *cm, float len);
void cmesh_draw_tangents(struct cmesh *cm, float len);

/* get the bounding box in local space. The result will be cached and subsequent
 * calls will return the same box. The cache gets invalidated by any functions that
 * can affect the vertex data
 */
void cmesh_aabbox(struct cmesh *cm, cgm_vec3 *vmin, cgm_vec3 *vmax);

/* get the bounding sphere in local space. The result will be cached ... see above */
float cmesh_bsphere(struct cmesh *cm, cgm_vec3 *center, float *rad);

/* texture coordinate manipulation */
void cmesh_texcoord_apply_xform(struct cmesh *cm, float *xform);
void cmesh_texcoord_gen_plane(struct cmesh *cm, cgm_vec3 *norm, cgm_vec3 *tang);
void cmesh_texcoord_gen_box(struct cmesh *cm);
void cmesh_texcoord_gen_cylinder(struct cmesh *cm);

int cmesh_dump(struct cmesh *cm, const char *fname);
int cmesh_dump_file(struct cmesh *cm, FILE *fp);
int cmesh_dump_obj(struct cmesh *cm, const char *fname);
int cmesh_dump_obj_file(struct cmesh *cm, FILE *fp, int voffs);
