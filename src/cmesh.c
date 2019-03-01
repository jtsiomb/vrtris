#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <assert.h>
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


static int pre_draw(struct cmesh *cm);
static void post_draw(struct cmesh *cm, int cur_sdr);
static void update_buffers(struct cmesh *cm);
static void update_wire_ibo(struct cmesh *cm);
static void calc_aabb(struct cmesh *cm);
static void calc_bsph(struct cmesh *cm);

static int def_nelem[CMESH_NUM_ATTR] = {3, 3, 3, 2, 4, 4, 4, 2};

static int sdr_loc[CMESH_NUM_ATTR] = {0, 1, 2, 3, 4, 5, 6, 7};
static int use_custom_sdr_attr;


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
		cm->vattr[i].vbo = cm->buffer_objects[i];
	}

	cm->ibo = cm->buffer_objects[CMESH_NUM_ATTR];
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
			cmdest->vattr[i].nelem = nelem;
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
	if(vdata) {
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

		glBindBuffer(GL_ARRAY_BUFFER, cm->vattr[attr].vbo);
		tmp = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
		memcpy(cm->vattr[attr].data, tmp, cm->nverts * nelem * sizeof(float));
		glUnmapBuffer(GL_ARRAY_BUFFER);

		cm->vattr[attr].data_valid = 1;
#endif
	}
	return cm->vattr[attr].data;
}

float *cmesh_attrib_at(struct cmesh *cm, int attr, int idx)
{
	float *vptr = cmesh_attrib(cm, attr);
	return vptr ? vptr + idx * cm->vattr[attr].nelem : 0;
}

const float *cmesh_attrib_at_ro(struct cmesh *cm, int attr, int idx)
{
	const float *vptr = cmesh_attrib_ro(cm, attr);
	return vptr ? vptr + idx * cm->vattr[attr].nelem : 0;
}

int cmesh_attrib_count(struct cmesh *cm, int attr)
{
	return cmesh_has_attrib(cm, attr) ? cm->nverts : 0;
}

int cmesh_push_attrib(struct cmesh *cm, int attr, float *v)
{
	float *vptr;
	int i, cursz, newsz;

	if(!cm->vattr[attr].nelem) {
		cm->vattr[attr].nelem = def_nelem[attr];
	}

	cursz = dynarr_size(cm->vattr[attr].data);
	newsz = cursz + cm->vattr[attr].nelem;
	if(!(vptr = dynarr_resize(cm->vattr[attr].data, newsz))) {
		return -1;
	}
	cm->vattr[attr].data = vptr;
	vptr += cursz;

	for(i=0; i<cm->vattr[attr].nelem; i++) {
		*vptr++ = *v++;
	}
	cm->vattr[attr].data_valid = 1;
	cm->vattr[attr].vbo_valid = 0;

	if(attr == CMESH_ATTR_VERTEX) {
		cm->nverts = newsz / cm->vattr[attr].nelem;
	}
	return 0;
}

int cmesh_push_attrib1f(struct cmesh *cm, int attr, float x)
{
	float v[4];
	v[0] = x;
	v[1] = v[2] = 0.0f;
	v[3] = 1.0f;
	return cmesh_push_attrib(cm, attr, v);
}

int cmesh_push_attrib2f(struct cmesh *cm, int attr, float x, float y)
{
	float v[4];
	v[0] = x;
	v[1] = y;
	v[2] = 0.0f;
	v[3] = 1.0f;
	return cmesh_push_attrib(cm, attr, v);
}

int cmesh_push_attrib3f(struct cmesh *cm, int attr, float x, float y, float z)
{
	float v[4];
	v[0] = x;
	v[1] = y;
	v[2] = z;
	v[3] = 1.0f;
	return cmesh_push_attrib(cm, attr, v);
}

int cmesh_push_attrib4f(struct cmesh *cm, int attr, float x, float y, float z, float w)
{
	float v[4];
	v[0] = x;
	v[1] = y;
	v[2] = z;
	v[3] = w;
	return cmesh_push_attrib(cm, attr, v);
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
#endif
	}
	return cm->idata;
}

int cmesh_index_count(struct cmesh *cm)
{
	return cm->nfaces * 3;
}

int cmesh_push_index(struct cmesh *cm, unsigned int idx)
{
	unsigned int *iptr;
	if(!(iptr = dynarr_push(cm->idata, &idx))) {
		return -1;
	}
	cm->idata = iptr;
	cm->idata_valid = 1;
	cm->ibo_valid = 0;

	cm->nfaces = dynarr_size(cm->idata) / 3;
	return 0;
}

int cmesh_poly_count(struct cmesh *cm)
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
	int i, nelem, newsz, origsz, srcsz;
	float *vptr;
	unsigned int *iptr;
	unsigned int idxoffs;

	if(!cmdest->nverts) {
		return cmesh_clone(cmdest, cmsrc);
	}

	for(i=0; i<CMESH_NUM_ATTR; i++) {
		if(cmesh_has_attrib(cmdest, i) && cmesh_has_attrib(cmsrc, i)) {
			/* force validation of the data arrays */
			cmesh_attrib(cmdest, i);
			cmesh_attrib_ro(cmsrc, i);

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

		idxoffs = cmdest->nverts;
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
	return 0;
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
				float *tmp = dynarr_push(cm->vattr[i].data, &cm->cur_val[i].x + j);
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

static float *get_vec4(struct cmesh *cm, int attr, int idx, cgm_vec4 *res)
{
	int i;
	float *sptr, *dptr;
	cgm_wcons(res, 0, 0, 0, 1);
	if(!(sptr = cmesh_attrib_at(cm, attr, idx))) {
		return 0;
	}
	dptr = &res->x;

	for(i=0; i<cm->vattr[attr].nelem; i++) {
		*dptr++ = sptr[i];
	}
	return sptr;
}

static float *get_vec3(struct cmesh *cm, int attr, int idx, cgm_vec3 *res)
{
	int i;
	float *sptr, *dptr;
	cgm_vcons(res, 0, 0, 0);
	if(!(sptr = cmesh_attrib_at(cm, attr, idx))) {
		return 0;
	}
	dptr = &res->x;

	for(i=0; i<cm->vattr[attr].nelem; i++) {
		*dptr++ = sptr[i];
	}
	return sptr;
}

/* dir_xform can be null, in which case it's calculated from xform */
void cmesh_apply_xform(struct cmesh *cm, float *xform, float *dir_xform)
{
	unsigned int i;
	int j;
	cgm_vec4 v;
	cgm_vec3 n, t;
	float *vptr;

	for(i=0; i<cm->nverts; i++) {
		if(!(vptr = get_vec4(cm, CMESH_ATTR_VERTEX, i, &v))) {
			return;
		}
		cgm_wmul_m4v4(&v, xform);
		for(j=0; j<cm->vattr[CMESH_ATTR_VERTEX].nelem; j++) {
			*vptr++ = (&v.x)[j];
		}

		if(cmesh_has_attrib(cm, CMESH_ATTR_NORMAL)) {
			if((vptr = get_vec3(cm, CMESH_ATTR_NORMAL, i, &n))) {
				cgm_vmul_m3v3(&n, dir_xform);
				for(j=0; j<cm->vattr[CMESH_ATTR_NORMAL].nelem; j++) {
					*vptr++ = (&n.x)[j];
				}
			}
		}
		if(cmesh_has_attrib(cm, CMESH_ATTR_TANGENT)) {
			if((vptr = get_vec3(cm, CMESH_ATTR_TANGENT, i, &t))) {
				cgm_vmul_m3v3(&t, dir_xform);
				for(j=0; j<cm->vattr[CMESH_ATTR_TANGENT].nelem; j++) {
					*vptr++ = (&t.x)[j];
				}
			}
		}
	}
}

void cmesh_flip(struct cmesh *cm)
{
	cmesh_flip_faces(cm);
	cmesh_flip_normals(cm);
}

void cmesh_flip_faces(struct cmesh *cm)
{
	int i, j, idxnum, vnum, nelem;
	unsigned int *indices;
	float *verts, *vptr;

	if(cmesh_indexed(cm)) {
		if(!(indices = cmesh_index(cm))) {
			return;
		}
		idxnum = cmesh_index_count(cm);
		for(i=0; i<idxnum; i+=3) {
			unsigned int tmp = indices[i + 2];
			indices[i + 2] = indices[i + 1];
			indices[i + 1] = tmp;
		}
	} else {
		if(!(verts = cmesh_attrib(cm, CMESH_ATTR_VERTEX))) {
			return;
		}
		vnum = cmesh_attrib_count(cm, CMESH_ATTR_VERTEX);
		nelem = cm->vattr[CMESH_ATTR_VERTEX].nelem;
		for(i=0; i<vnum; i+=3) {
			for(j=0; j<nelem; j++) {
				vptr = verts + (i + 1) * nelem + j;
				float tmp = vptr[nelem];
				vptr[nelem] = vptr[0];
				vptr[0] = tmp;
			}
		}
	}
}
void cmesh_flip_normals(struct cmesh *cm)
{
	int i, num;
	float *nptr = cmesh_attrib(cm, CMESH_ATTR_NORMAL);
	if(!nptr) return;

	num = cm->nverts * cm->vattr[CMESH_ATTR_NORMAL].nelem;
	for(i=0; i<num; i++) {
		*nptr = -*nptr;
		nptr++;
	}
}

int cmesh_explode(struct cmesh *cm)
{
	int i, j, k, idxnum, nnverts;
	unsigned int *indices;

	if(!cmesh_indexed(cm)) return 0;

	indices = cmesh_index(cm);
	assert(indices);

	idxnum = cmesh_index_count(cm);
	nnverts = idxnum;

	for(i=0; i<CMESH_NUM_ATTR; i++) {
		const float *srcbuf;
		float *tmpbuf, *dstptr;

		if(!cmesh_has_attrib(cm, i)) continue;

		srcbuf = cmesh_attrib(cm, i);
		if(!(tmpbuf = dynarr_alloc(nnverts * cm->vattr[i].nelem, sizeof(float)))) {
			return -1;
		}
		dstptr = tmpbuf;

		for(j=0; j<idxnum; j++) {
			unsigned int idx = indices[j];
			const float *srcptr = srcbuf + idx * cm->vattr[i].nelem;

			for(k=0; k<cm->vattr[i].nelem; k++) {
				*dstptr++ = *srcptr++;
			}
		}

		dynarr_free(cm->vattr[i].data);
		cm->vattr[i].data = tmpbuf;
		cm->vattr[i].data_valid = 1;
	}

	cm->ibo_valid = 0;
	cm->idata_valid = 0;
	cm->idata = dynarr_clear(cm->idata);

	cm->nverts = nnverts;
	cm->nfaces = idxnum / 3;
	return 0;
}

void cmesh_calc_face_normals(struct cmesh *cm)
{
	/* TODO */
}

static int pre_draw(struct cmesh *cm)
{
	int i, loc, cur_sdr;

	glGetIntegerv(GL_CURRENT_PROGRAM, &cur_sdr);

	update_buffers(cm);

	if(!cm->vattr[CMESH_ATTR_VERTEX].vbo_valid) {
		return -1;
	}

	if(cur_sdr && use_custom_sdr_attr) {
		if(sdr_loc[CMESH_ATTR_VERTEX] == -1) {
			return -1;
		}

		for(i=0; i<CMESH_NUM_ATTR; i++) {
			loc = sdr_loc[i];
			if(loc >= 0 && cm->vattr[i].vbo_valid) {
				glBindBuffer(GL_ARRAY_BUFFER, cm->vattr[i].vbo);
				glVertexAttribPointer(loc, cm->vattr[i].nelem, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(loc);
			}
		}
	} else {
#ifndef GL_ES_VERSION_2_0
		glBindBuffer(GL_ARRAY_BUFFER, cm->vattr[CMESH_ATTR_VERTEX].vbo);
		glVertexPointer(cm->vattr[CMESH_ATTR_VERTEX].nelem, GL_FLOAT, 0, 0);
		glEnableClientState(GL_VERTEX_ARRAY);

		if(cm->vattr[CMESH_ATTR_NORMAL].vbo_valid) {
			glBindBuffer(GL_ARRAY_BUFFER, cm->vattr[CMESH_ATTR_NORMAL].vbo);
			glNormalPointer(GL_FLOAT, 0, 0);
			glEnableClientState(GL_NORMAL_ARRAY);
		}
		if(cm->vattr[CMESH_ATTR_TEXCOORD].vbo_valid) {
			glBindBuffer(GL_ARRAY_BUFFER, cm->vattr[CMESH_ATTR_TEXCOORD].vbo);
			glTexCoordPointer(cm->vattr[CMESH_ATTR_TEXCOORD].nelem, GL_FLOAT, 0, 0);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		if(cm->vattr[CMESH_ATTR_COLOR].vbo_valid) {
			glBindBuffer(GL_ARRAY_BUFFER, cm->vattr[CMESH_ATTR_COLOR].vbo);
			glColorPointer(cm->vattr[CMESH_ATTR_COLOR].nelem, GL_FLOAT, 0, 0);
			glEnableClientState(GL_COLOR_ARRAY);
		}
		if(cm->vattr[CMESH_ATTR_TEXCOORD2].vbo_valid) {
			glClientActiveTexture(GL_TEXTURE1);
			glBindBuffer(GL_ARRAY_BUFFER, cm->vattr[CMESH_ATTR_TEXCOORD2].vbo);
			glTexCoordPointer(cm->vattr[CMESH_ATTR_TEXCOORD2].nelem, GL_FLOAT, 0, 0);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glClientActiveTexture(GL_TEXTURE0);
		}
#endif	/* GL_ES_VERSION_2_0 */
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return cur_sdr;
}

void cmesh_draw(struct cmesh *cm)
{
	int cur_sdr;

	if((cur_sdr = pre_draw(cm)) == -1) {
		return;
	}

	if(cm->ibo_valid) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cm->ibo);
		glDrawElements(GL_TRIANGLES, cm->nfaces * 3, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	} else {
		glDrawArrays(GL_TRIANGLES, 0, cm->nverts);
	}

	post_draw(cm, cur_sdr);
}

static void post_draw(struct cmesh *cm, int cur_sdr)
{
	int i;

	if(cur_sdr && use_custom_sdr_attr) {
		for(i=0; i<CMESH_NUM_ATTR; i++) {
			int loc = sdr_loc[i];
			if(loc >= 0 && cm->vattr[i].vbo_valid) {
				glDisableVertexAttribArray(loc);
			}
		}
	} else {
#ifndef GL_ES_VERSION_2_0
		glDisableClientState(GL_VERTEX_ARRAY);
		if(cm->vattr[CMESH_ATTR_NORMAL].vbo_valid) {
			glDisableClientState(GL_NORMAL_ARRAY);
		}
		if(cm->vattr[CMESH_ATTR_TEXCOORD].vbo_valid) {
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		if(cm->vattr[CMESH_ATTR_COLOR].vbo_valid) {
			glDisableClientState(GL_COLOR_ARRAY);
		}
		if(cm->vattr[CMESH_ATTR_TEXCOORD2].vbo_valid) {
			glClientActiveTexture(GL_TEXTURE1);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glClientActiveTexture(GL_TEXTURE0);
		}
#endif	/* GL_ES_VERSION_2_0 */
	}
}

void cmesh_draw_wire(struct cmesh *cm, float linesz)
{
	int cur_sdr, nfaces;

	if((cur_sdr = pre_draw(cm)) == -1) {
		return;
	}
	update_wire_ibo(cm);

	nfaces = cmesh_poly_count(cm);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cm->wire_ibo);
	glDrawElements(GL_LINES, nfaces * 6, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	post_draw(cm, cur_sdr);
}

void cmesh_draw_vertices(struct cmesh *cm, float ptsz)
{
	int cur_sdr;
	if((cur_sdr = pre_draw(cm)) == -1) {
		return;
	}

	glPushAttrib(GL_POINT_BIT);
	glPointSize(ptsz);
	glDrawArrays(GL_POINTS, 0, cm->nverts);
	glPopAttrib();

	post_draw(cm, cur_sdr);
}

void cmesh_draw_normals(struct cmesh *cm, float len)
{
#ifndef GL_ES_VERSION_2_0
	int i, cur_sdr, vert_nelem, norm_nelem;
	int loc = -1;
	const float *varr, *norm;

	varr = cmesh_attrib_ro(cm, CMESH_ATTR_VERTEX);
	norm = cmesh_attrib_ro(cm, CMESH_ATTR_NORMAL);
	if(!varr || !norm) return;

	vert_nelem = cm->vattr[CMESH_ATTR_VERTEX].nelem;
	norm_nelem = cm->vattr[CMESH_ATTR_NORMAL].nelem;

	glGetIntegerv(GL_CURRENT_PROGRAM, &cur_sdr);
	if(cur_sdr && use_custom_sdr_attr) {
		if((loc = sdr_loc[CMESH_ATTR_VERTEX]) < 0) {
			return;
		}
	}

	glBegin(GL_LINES);
	for(i=0; i<cm->nverts; i++) {
		float x, y, z, endx, endy, endz;

		x = varr[i * vert_nelem];
		y = varr[i * vert_nelem + 1];
		z = varr[i * vert_nelem + 2];
		endx = x + norm[i * norm_nelem] * len;
		endy = y + norm[i * norm_nelem + 1] * len;
		endz = z + norm[i * norm_nelem + 2] * len;

		if(loc == -1) {
			glVertex3f(x, y, z);
			glVertex3f(endx, endy, endz);
		} else {
			glVertexAttrib3f(loc, x, y, z);
			glVertexAttrib3f(loc, endx, endy, endz);
		}
	}
	glEnd();
#endif	/* GL_ES_VERSION_2_0 */
}

void cmesh_draw_tangents(struct cmesh *cm, float len)
{
#ifndef GL_ES_VERSION_2_0
	int i, cur_sdr, vert_nelem, tang_nelem;
	int loc = -1;
	const float *varr, *tang;

	varr = cmesh_attrib_ro(cm, CMESH_ATTR_VERTEX);
	tang = cmesh_attrib_ro(cm, CMESH_ATTR_TANGENT);
	if(!varr || !tang) return;

	vert_nelem = cm->vattr[CMESH_ATTR_VERTEX].nelem;
	tang_nelem = cm->vattr[CMESH_ATTR_TANGENT].nelem;

	glGetIntegerv(GL_CURRENT_PROGRAM, &cur_sdr);
	if(cur_sdr && use_custom_sdr_attr) {
		if((loc = sdr_loc[CMESH_ATTR_VERTEX]) < 0) {
			return;
		}
	}

	glBegin(GL_LINES);
	for(i=0; i<cm->nverts; i++) {
		float x, y, z, endx, endy, endz;

		x = varr[i * vert_nelem];
		y = varr[i * vert_nelem + 1];
		z = varr[i * vert_nelem + 2];
		endx = x + tang[i * tang_nelem] * len;
		endy = y + tang[i * tang_nelem + 1] * len;
		endz = z + tang[i * tang_nelem + 2] * len;

		if(loc == -1) {
			glVertex3f(x, y, z);
			glVertex3f(endx, endy, endz);
		} else {
			glVertexAttrib3f(loc, x, y, z);
			glVertexAttrib3f(loc, endx, endy, endz);
		}
	}
	glEnd();
#endif	/* GL_ES_VERSION_2_0 */
}

static void update_buffers(struct cmesh *cm)
{
	int i;

	for(i=0; i<CMESH_NUM_ATTR; i++) {
		if(cmesh_has_attrib(cm, i) && !cm->vattr[i].vbo_valid) {
			glBindBuffer(GL_ARRAY_BUFFER, cm->vattr[i].vbo);
			glBufferData(GL_ARRAY_BUFFER, cm->nverts * cm->vattr[i].nelem * sizeof(float),
					cm->vattr[i].data, GL_STATIC_DRAW);
			cm->vattr[i].vbo_valid = 1;
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if(cm->idata_valid && !cm->ibo_valid) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cm->ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, cm->nfaces * 3 * sizeof(unsigned int),
				cm->idata, GL_STATIC_DRAW);
		cm->ibo_valid = 1;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

static void update_wire_ibo(struct cmesh *cm)
{
	int i, num_faces;
	unsigned int *wire_idxarr, *dest;

	update_buffers(cm);

	if(cm->wire_ibo_valid) return;

	if(!cm->wire_ibo) {
		glGenBuffers(1, &cm->wire_ibo);
	}
	num_faces = cmesh_poly_count(cm);

	if(!(wire_idxarr = malloc(num_faces * 6 * sizeof *wire_idxarr))) {
		return;
	}
	dest = wire_idxarr;

	if(cm->ibo_valid) {
		/* we're dealing with an indexed mesh */
		const unsigned int *idxarr = cmesh_index_ro(cm);

		for(i=0; i<num_faces; i++) {
			*dest++ = idxarr[0];
			*dest++ = idxarr[1];
			*dest++ = idxarr[1];
			*dest++ = idxarr[2];
			*dest++ = idxarr[2];
			*dest++ = idxarr[0];
			idxarr += 3;
		}
	} else {
		/* not an indexed mesh */
		for(i=0; i<num_faces; i++) {
			int vidx = i * 3;
			*dest++ = vidx;
			*dest++ = vidx + 1;
			*dest++ = vidx + 1;
			*dest++ = vidx + 2;
			*dest++ = vidx + 2;
			*dest++ = vidx;
		}
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cm->wire_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_faces * 6 * sizeof(unsigned int),
			wire_idxarr, GL_STATIC_DRAW);
	free(wire_idxarr);
	cm->wire_ibo_valid = 1;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static void calc_aabb(struct cmesh *cm)
{
	int i, j;

	if(!cmesh_attrib_ro(cm, CMESH_ATTR_VERTEX)) {
		return;
	}

	cgm_vcons(&cm->aabb_min, FLT_MAX, FLT_MAX, FLT_MAX);
	cgm_vcons(&cm->aabb_max, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	for(i=0; i<cm->nverts; i++) {
		const float *v = cmesh_attrib_at_ro(cm, CMESH_ATTR_VERTEX, i);
		for(j=0; j<3; j++) {
			if(v[j] < (&cm->aabb_min.x)[j]) {
				(&cm->aabb_min.x)[j] = v[j];
			}
			if(v[j] > (&cm->aabb_max.x)[j]) {
				(&cm->aabb_max.x)[j] = v[j];
			}
		}
	}
	cm->aabb_valid = 1;
}

void cmesh_aabbox(struct cmesh *cm, cgm_vec3 *vmin, cgm_vec3 *vmax)
{
	if(!cm->aabb_valid) {
		calc_aabb(cm);
	}
	*vmin = cm->aabb_min;
	*vmax = cm->aabb_max;
}

static void calc_bsph(struct cmesh *cm)
{
	int i;
	float s, dist_sq;

	if(!cmesh_attrib_ro(cm, CMESH_ATTR_VERTEX)) {
		return;
	}

	cgm_vcons(&cm->bsph_center, 0, 0, 0);

	/* first find the center */
	for(i=0; i<cm->nverts; i++) {
		const float *v = cmesh_attrib_at_ro(cm, CMESH_ATTR_VERTEX, i);
		cm->bsph_center.x += v[0];
		cm->bsph_center.y += v[1];
		cm->bsph_center.z += v[2];
	}
	s = 1.0f / (float)cm->nverts;
	cm->bsph_center.x *= s;
	cm->bsph_center.y *= s;
	cm->bsph_center.z *= s;

	cm->bsph_radius = 0.0f;
	for(i=0; i<cm->nverts; i++) {
		const cgm_vec3 *v = (const cgm_vec3*)cmesh_attrib_at_ro(cm, CMESH_ATTR_VERTEX, i);
		if((dist_sq = cgm_vdist_sq(v, &cm->bsph_center)) > cm->bsph_radius) {
			cm->bsph_radius = dist_sq;
		}
	}
	cm->bsph_radius = sqrt(cm->bsph_radius);
	cm->bsph_valid = 1;
}

float cmesh_bsphere(struct cmesh *cm, cgm_vec3 *center, float *rad)
{
	if(!cm->bsph_valid) {
		calc_bsph(cm);
	}
	*center = cm->bsph_center;
	*rad = cm->bsph_radius;
	return cm->bsph_radius;
}

/* TODO */
void cmesh_texcoord_apply_xform(struct cmesh *cm, float *xform);
void cmesh_texcoord_gen_plane(struct cmesh *cm, cgm_vec3 *norm, cgm_vec3 *tang);
void cmesh_texcoord_gen_box(struct cmesh *cm);
void cmesh_texcoord_gen_cylinder(struct cmesh *cm);

int cmesh_dump(struct cmesh *cm, const char *fname);
int cmesh_dump_file(struct cmesh *cm, FILE *fp);
int cmesh_dump_obj(struct cmesh *cm, const char *fname);
int cmesh_dump_obj_file(struct cmesh *cm, FILE *fp, int voffs);
