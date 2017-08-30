
#ifndef DIRECTORY_H_
#define DIRECTORY_H_

struct directory_entry_t{
	unsigned int p0 : 1;
	unsigned int p1 : 1;
	unsigned int p2 : 1;
	unsigned int p3 : 1;
	unsigned int p4 : 1;
	unsigned int p5 : 1;
	unsigned int p6 : 1;
	unsigned int p7 : 1;

	unsigned int p8 : 1;
	unsigned int p9 : 1;
	unsigned int p10 : 1;
	unsigned int p11 : 1;
	unsigned int p12 : 1;
	unsigned int p13 : 1;
	unsigned int p14 : 1;
	unsigned int p15 : 1;

	unsigned int p16 : 1;
	unsigned int p17 : 1;
	unsigned int p18 : 1;
	unsigned int p19 : 1;
	unsigned int p20 : 1;
	unsigned int p21 : 1;
	unsigned int p22 : 1;
	unsigned int p23 : 1;

	unsigned int p24 : 1;
	unsigned int p25 : 1;
	unsigned int p26 : 1;
	unsigned int p27 : 1;
	unsigned int p28 : 1;
	unsigned int p29 : 1;
	unsigned int p30 : 1;
	unsigned int p31 : 1;

	unsigned int p32 : 1;
	unsigned int p33 : 1;
	unsigned int p34 : 1;
	unsigned int p35 : 1;
	unsigned int p36 : 1;
	unsigned int p37 : 1;
	unsigned int p38 : 1;
	unsigned int p39 : 1;

	unsigned int p40 : 1;
	unsigned int p41 : 1;
	unsigned int p42 : 1;
	unsigned int p43 : 1;
	unsigned int p44 : 1;
	unsigned int p45 : 1;
	unsigned int p46 : 1;
	unsigned int p47 : 1;

	unsigned int p48 : 1;
	unsigned int p49 : 1;
	unsigned int p50 : 1;
	unsigned int p51 : 1;
	unsigned int p52 : 1;
	unsigned int p53 : 1;
	unsigned int p54 : 1;
	unsigned int p55 : 1;

	unsigned int p56 : 1;
	unsigned int p57 : 1;
	unsigned int p58 : 1;
	unsigned int p59 : 1;
	unsigned int p60 : 1;
	/*unsigned int p61 : 1;
	unsigned int p62 : 1;
	unsigned int p63 : 1;*/

	unsigned int dirty : 1;
	unsigned int pending : 1;
	unsigned int upgrade : 1;
};

union directory_t{

	unsigned long long entry;
	struct directory_entry_t entry_bits;
};

extern unsigned int dir_mode; //1 = scoc mode 0 equals system agent mode
extern unsigned long long dir_mem_image_size;
extern unsigned int dir_block_size;
extern unsigned long long dir_num_blocks;
extern unsigned int dir_block_mask;
extern unsigned int dir_vector_size;


#endif /*DIRECTORY_H_*/
