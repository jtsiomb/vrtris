#ifndef BLOCKS_H_
#define BLOCKS_H_

#define BLK(x, y)	((x) | ((y) << 4))
#define BLKX(c)		((unsigned char)(c) & 0xf)
#define BLKY(c)		((unsigned char)(c) >> 4)

#define NUM_BLOCKS	7

static unsigned char blocks[NUM_BLOCKS][4][4] = {
	/* L block */
	{
		{BLK(0, 1), BLK(0, 2), BLK(1, 1), BLK(2, 1)},
		{BLK(0, 0), BLK(1, 0), BLK(1, 1), BLK(1, 2)},
		{BLK(0, 1), BLK(1, 1), BLK(2, 1), BLK(2, 0)},
		{BLK(1, 0), BLK(1, 1), BLK(1, 2), BLK(2, 2)}
	},
	/* J block */
	{
		{BLK(0, 1), BLK(1, 1), BLK(2, 1), BLK(2, 2)},
		{BLK(1, 0), BLK(1, 1), BLK(1, 2), BLK(0, 2)},
		{BLK(0, 0), BLK(0, 1), BLK(1, 1), BLK(2, 1)},
		{BLK(1, 0), BLK(2, 0), BLK(1, 1), BLK(1, 2)}
	},
	/* I block */
	{
		{BLK(0, 2), BLK(1, 2), BLK(2, 2), BLK(3, 2)},
		{BLK(1, 0), BLK(1, 1), BLK(1, 2), BLK(1, 3)},
		{BLK(0, 2), BLK(1, 2), BLK(2, 2), BLK(3, 2)},
		{BLK(1, 0), BLK(1, 1), BLK(1, 2), BLK(1, 3)}
	},
	/* O block */
	{
		{BLK(1, 1), BLK(2, 1), BLK(1, 2), BLK(2, 2)},
		{BLK(1, 1), BLK(2, 1), BLK(1, 2), BLK(2, 2)},
		{BLK(1, 1), BLK(2, 1), BLK(1, 2), BLK(2, 2)},
		{BLK(1, 1), BLK(2, 1), BLK(1, 2), BLK(2, 2)}
	},
	/* Z block */
	{
		{BLK(0, 1), BLK(1, 1), BLK(1, 2), BLK(2, 2)},
		{BLK(0, 1), BLK(1, 1), BLK(1, 0), BLK(0, 2)},
		{BLK(0, 1), BLK(1, 1), BLK(1, 2), BLK(2, 2)},
		{BLK(0, 1), BLK(1, 1), BLK(1, 0), BLK(0, 2)}
	},
	/* S block */
	{
		{BLK(1, 1), BLK(2, 1), BLK(0, 2), BLK(1, 2)},
		{BLK(0, 0), BLK(0, 1), BLK(1, 1), BLK(1, 2)},
		{BLK(1, 1), BLK(2, 1), BLK(0, 2), BLK(1, 2)},
		{BLK(0, 0), BLK(0, 1), BLK(1, 1), BLK(1, 2)}
	},
	/* T block */
	{
		{BLK(0, 1), BLK(1, 1), BLK(2, 1), BLK(1, 2)},
		{BLK(1, 0), BLK(1, 1), BLK(1, 2), BLK(0, 1)},
		{BLK(0, 1), BLK(1, 1), BLK(2, 1), BLK(1, 0)},
		{BLK(1, 0), BLK(1, 1), BLK(1, 2), BLK(2, 1)}
	}
};

static int block_spawnpos[NUM_BLOCKS][2] = {
	{-1, -2}, {-1, -2}, {-2, -2}, {-1, -2}, {-1, -2}, {-1, -2}, {-1, -2}
};


#endif	/* BLOCKS_H_ */
