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
	cgm_clear(cm);

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
	memset(cm, 0, sizeof *cm);

	cgm_wcons(cm->cur_val + CMESH_ATTR_COLOR, 1, 1, 1, 1);
	/* TODO */
}

void cmesh_copy(struct cmesh *cmdest, struct cmesh *cmsrc);

void cmesh_set_name(struct cmesh *cm, const char *name);
const char *cmesh_name(struct cmesh *cm);

int cmesh_has_attrib(struct cmesh *cm, int attr);
int cmesh_indexed(struct cmesh *cm);

/* vdata can be 0, in which case only memory is allocated
 * returns pointer to the attribute array
 */
float *cmesh_set_attrib(struct cmesh *cm, int attr, int nelem, unsigned int num,
		const float *vdata);
float *cmesh_attrib(struct cmesh *cm, int attr);	/* invalidates VBO */
const float *cmesh_attrib_ro(struct cmesh *cm, int attr);	/* doesn't invalidate */
int cmesh_attrib_count(struct cmesh *cm, int attr);

/* indices can be 0, in which case only memory is allocated
 * returns pointer to the index array
 */
unsigned int *cmesh_set_index(struct cmesh *cm, int num, const unsigned int *indices);
unsigned int *cmesh_index(struct cmesh *cm);	/* invalidates IBO */
const unsigned int *cmesh_index_ro(struct cmesh *cm);	/* doesn't invalidate */
int cmesh_index_count(struct cmesh *cm);

int get_poly_count(struct cmesh *cm);

/* attr can be -1 to invalidate all attributes */
void cmesh_invalidate_attrib(struct cmesh *cm, int attr);
void cmesh_invalidate_index(struct cmesh *cm);

int cmesh_append(struct cmesh *cmdest, struct cmesh *cmsrc);

/* immediate-mode style mesh construction interface */
int cmesh_vertex(struct cmesh *cm, float x, float y, float z);
int cmesh_normal(struct cmesh *cm, float nx, float ny, float nz);
int cmesh_tangent(struct cmesh *cm, float tx, float ty, float tz);
int cmesh_texcoord(struct cmesh *cm, float u, float v, float w);
int cmesh_boneweights(struct cmesh *cm, float w1, float w2, float w3, float w4);
int cmesh_boneidx(struct cmesh *cm, int idx1, int idx2, int idx3, int idx4);

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
