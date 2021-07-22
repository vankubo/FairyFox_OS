#include "memfox.h"
#include <stdio.h>
//bank info
static uint8_t *Bank_start;
static uint8_t *Bank_end;
static uint32_t Bank_size;
static uint32_t Bank_used;
static uint8_t *Mem_start;
static uint32_t Cell_list_len;
static uint32_t Cell_list_used_num;
static Cell_head **Cell_list;

/*
tools
*/
static void memfox_memset(uint8_t *start,uint32_t size,uint8_t val)
{
    int i;
    for(i=0;i<size;i++)
    {
        start[i]=val;
    }
}



/*
get gap size of 2 cells
always return positive value
*/
static int memfox_getGap(Cell_head *cell_a,Cell_head *cell_b)
{
    int gap=0;
    uint32_t cellsize=0;
    if(cell_a>cell_b)
    {
        cellsize=cell_b->cell_szie;
        gap=(uint8_t*)cell_a-(uint8_t*)(cell_b+cellsize);
    }
    else if(cell_a<cell_b)
    {
        cellsize=cell_a->cell_szie;
        gap=(uint8_t*)cell_b-(uint8_t*)(cell_a+cellsize);
    }
    return gap;
}

/*
find the most fit cell index
return
-1-fit memstart
n-fit n
*/
static uint32_t memfox_findFistIndex(uint32_t size)
{
    int gap=0;
    int memdiff=0;
    int mindiff=DEFAULT_CELL_LEN;
    int fistindex;
    int i;


	fistindex = Cell_list_used_num - 1;
	//get fist cell addr
	for (i = 0; i < Cell_list_used_num; i++)
	{
		if (i == 0)
		{
			gap = (uint8_t*)Cell_list[i] - (uint8_t*)(Cell_head*)Mem_start;
		}
		else if (i > 0)
		{
			gap = memfox_getGap(Cell_list[i - 1], Cell_list[i]);
		}


		memdiff = gap - (size);
		if (memdiff >= 0)
		{
			if (memdiff <= mindiff)
			{
				mindiff = memdiff;
				if (i == 0)
					fistindex = -1;
				else
				{
					if (Cell_list[i] > Cell_list[i - 1])
						fistindex = i - 1;
					else
						fistindex = i;
				}

			}
		}
	}

    return fistindex;
}

/*
find cell index from mem pointer
*/
static uint32_t memfox_findCellIndex(void *ptr)
{
    int i;
    uint8_t *memp=NULL;
    for(i=0;i<Cell_list_used_num;i++)
    {
        memp=(uint8_t*)Cell_list[i]+sizeof(Cell_head);
		if (memp == (uint8_t*)ptr)
			break; 
    }
    return i;
}

//----------------------------------------------------------------------------------
/*
init memory info and memfox
BankStart must align with 4bytes
*/
uint8_t memfox_init(uint8_t *BankStart,uint32_t BankSize)
{
	int i;
    uint32_t CellNum;
    //check bank size
    if(BankSize<=(DEFAULT_CELL_LEN))
        return -1;
    //get bank info
    Bank_start=BankStart;
    Bank_size=BankSize;
    Bank_end=BankStart+BankSize;
    Bank_used=0;
    Cell_list=BankStart;
    
    //set bank unit value
    memfox_memset((uint8_t *)BankStart,BankSize,UNUSED_UNIT);
   
    //get cell list len
    CellNum=BankSize/DEFAULT_CELL_LEN;
    Cell_list_len=sizeof(Cell_head)*CellNum;
    Mem_start=(BankStart+Cell_list_len);
    Cell_list_used_num=0;
    Bank_used+=Cell_list_len;
    return 0;
}

/*
malloc
*/
void* memfox_malloc(uint32_t size)
{
    int i,fistindex;
	uint32_t gap;
    int addbytes=0;
    uint8_t *tailaddr=NULL;
    Cell_head *celladdr=NULL;


	//check for safe
    if(size<=0)
        return (void*)NULL;
	if (size >= (Bank_size - Bank_used))
		return (void*)NULL;

	//get new cell addr
	//first malloc
	if (Cell_list_used_num == 0)
	{
		celladdr = (Cell_head*)Mem_start;
	}
	else
	{
		//find fist index
		fistindex = memfox_findFistIndex(size);
		//printf("[memfox]fistindex=%d\n", fistindex);
		if (fistindex == -1)
			celladdr = (Cell_head*)Mem_start;
		else
			celladdr = ((uint8_t*)Cell_list[fistindex] + Cell_list[fistindex]->cell_szie);
	}
	
		

	//printf("[memfox]get new cell start addr done:0x%X\n", celladdr);
	//set cell list
	Cell_list[Cell_list_used_num] = celladdr;

	//set cell head
	celladdr->cell_head_flag = DEFAULT_CELL_STA;
	celladdr->cell_tail_flag = DEFAULT_CELL_END;
	celladdr->mem_size = size;
	addbytes = (int)(celladdr+size) % 4;
	if (addbytes>0)
	{
		addbytes = 4 - addbytes;
	}
	else if (addbytes == 0)
	{
		addbytes = 4;
	}
	
	//set cell tail
	tailaddr = ((uint8_t*)celladdr + sizeof(Cell_head) + size);
	for (i = 0; i<addbytes; i++)
	{
		tailaddr[i] = DEFAULT_CELL_END;
	}

	celladdr->cell_szie = sizeof(Cell_head) + size + addbytes;
	celladdr->cell_tail_size = addbytes;
	
	//set environment
	Cell_list_used_num++;
	Bank_used += celladdr->cell_szie;
    
    //return cell mem addr
    return (void *)((uint8_t*)celladdr+sizeof(Cell_head));
}



void memfox_free(void *ptr)
{
    int i;
    uint32_t cellindex=0;
    uint32_t cellsize=0;
    //find cell
    cellindex=memfox_findCellIndex(ptr);
	//printf("[fox free]cellindex=%d\n", cellindex);

    //clear cell
    cellsize=Cell_list[cellindex]->cell_szie;
    memfox_memset((uint8_t *)Cell_list[cellindex],cellsize,UNUSED_UNIT);
    
    //delete cell from cell list and rerange cell list
    for(i=cellindex;i<Cell_list_used_num;i++)
    {
        Cell_list[i]=Cell_list[i+1];
    }
    //set environment
    Cell_list_used_num--;
    Bank_used-=cellsize;
}


uint32_t memfox_GetBankUsed(void)
{
	return Bank_used;
}

uint32_t memfox_GetBankTotal(void)
{
	return Bank_size;
}

uint32_t memfox_GetListIndex(void)
{
	return Cell_list_used_num;
}

uint32_t memfox_GetLiseLen(void)
{
	return Cell_list_len;
}

/*
void memfox_printlist(void)
{
	int i;
	printf("\n====>\n");
	for (i = 0; i < Cell_list_used_num; i++)
	{
		printf("list[%d]->0x%X\n", i, Cell_list[i]);
	}
	printf("\n====\n");
}
*/