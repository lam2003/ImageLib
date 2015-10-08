#pragma once
#include "itcdatastructs.h"
#include "limits.h"
#include "itcCore.h"
#include <assert.h>
#include <iostream>
#include <stdlib.h>
#include <malloc.h>


#define cvFree(ptr) (cvFree_(*(ptr)), *(ptr)=0)

/* maximum size of dynamic memory buffer.
   cvAlloc reports an error if a larger block is requested. */
#define  ITC_MAX_ALLOC_SIZE    (((size_t)1 << (sizeof(size_t)*8-2)))  //判断是否越界
/* the alignment of all the allocated buffers */
#define  ITC_MALLOC_ALIGN    32
/* default storage block size */
#define  ITC_STORAGE_BLOCK_SIZE   ((1<<16) - 128)
/* default alignment for dynamic data strucutures, resided in storages. */
#define  ITC_STRUCT_ALIGN    ((int)sizeof(double))

#define ITC_GET_LAST_ELEM( seq, block ) \
	((block)->data + ((block)->count - 1)*((seq)->elem_size))

#define ITC_FREE_PTR(storage)  \
	((char*)(storage)->top + (storage)->block_size - (storage)->free_space)
/* 0x3a50 = 11 10 10 01 01 00 00 ~ array of log2(sizeof(arr_type_elem)) */
#define ITC_ELEM_SIZE(type) \
	(ITC_MAT_CN(type) << ((((sizeof(size_t)/4+1)*16384|0x3a50) >> ITC_MAT_DEPTH(type)*2) & 3))
#define ITC_SHIFT_TAB_MAX 32
static const char itcPower2ShiftTab[] =
{
	0, 1, -1, 2, -1, -1, -1, 3, -1, -1, -1, -1, -1, -1, -1, 4,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 5
};

#define ITC_MEMCPY_AUTO( dst, src, len )                                             \
{                                                                                   \
	size_t _icv_memcpy_i_, _icv_memcpy_len_ = (len);                                \
	char* _icv_memcpy_dst_ = (char*)(dst);                                          \
	const char* _icv_memcpy_src_ = (const char*)(src);                              \
if ((_icv_memcpy_len_ & (sizeof(int)-1)) == 0)                                 \
{                                                                               \
	assert(((size_t)_icv_memcpy_src_&(sizeof(int)-1)) == 0 && \
	((size_t)_icv_memcpy_dst_&(sizeof(int)-1)) == 0);                  \
for (_icv_memcpy_i_ = 0; _icv_memcpy_i_ < _icv_memcpy_len_;                 \
	_icv_memcpy_i_ += sizeof(int))                                           \
{                                                                           \
	*(int*)(_icv_memcpy_dst_ + _icv_memcpy_i_) = \
	*(const int*)(_icv_memcpy_src_ + _icv_memcpy_i_);                         \
	}                                                                           \
	}                                                                               \
	else                                                                            \
{                                                                               \
for (_icv_memcpy_i_ = 0; _icv_memcpy_i_ < _icv_memcpy_len_; _icv_memcpy_i_++)\
	_icv_memcpy_dst_[_icv_memcpy_i_] = _icv_memcpy_src_[_icv_memcpy_i_];    \
	}                                                                               \
}


//error code
#define  ITC_OK 0
#define  ITC_StsBadSize    -201 /* the input/output structure size is incorrect  */
#define  ITC_StsOutOfRange -211  /* some of parameters are out of range */
#define  ITC_StsNoMem -4	/* insufficient memory */
#define  ITC_StsNullPtr  -27 /* null pointer */
#define  ITC_BADARG_ERR   -49  //ipp comp

/* the alignment of all the allocated buffers */
#define  ITC_MALLOC_ALIGN    32
// pointers to allocation functions, initially set to default
static void* p_cvAllocUserData = 0;



inline int  itcAlign(int size, int align)
{
	assert((align&(align-1))==0 && size<INT_MAX);
	return (size + align - 1) & -align;
}

inline void* itcAlignPtr( const void* ptr, int align=32 )
{
	assert( (align & (align-1)) == 0 );
	return (void*)( ((size_t)ptr + align - 1) & ~(size_t)(align-1) );
}

inline int
	itcAlignLeft( int size, int align )
{
	return size & -align;
}



// default <malloc>
static void*
	itcDefaultAlloc( size_t size, void* )
{
	char *ptr, *ptr0 = (char*)malloc(
		(size_t)(size + ITC_MALLOC_ALIGN*((size >= 4096) + 1) + sizeof(char*)));   //多申请了 ITC_MALLOC_ALIGN*((size >= 4096) + 1) + sizeof(char*)大小的内存

	if( !ptr0 )
		return 0;

	// align the pointer
	ptr = (char*)itcAlignPtr(ptr0 + sizeof(char*) + 1, ITC_MALLOC_ALIGN);   //将指针对齐到ITC_MALLOC_ALIGN，32bit既4个字节，将指针调整到32的整数倍
	*(char**)(ptr - sizeof(char*)) = ptr0;	//将ptr0记录到(ptr – sizeof(char*))

	return ptr;
}

// default <free>
static int
	itcDefaultFree( void* ptr, void* )
{
	// Pointer must be aligned by CV_MALLOC_ALIGN
	if( ((size_t)ptr & (ITC_MALLOC_ALIGN-1)) != 0 )
		return ITC_BADARG_ERR;
	free( *((char**)ptr - 1) );

	return ITC_OK;
}

// pointers to allocation functions, initially set to default
//static CvAllocFunc p_cvAlloc = icvDefaultAlloc;

void*  itcAlloc( size_t size )
{
	void* ptr = 0;

	//CV_FUNCNAME( "cvAlloc" );

	//__BEGIN__;

	if( (size_t)size > ITC_MAX_ALLOC_SIZE )
		std::cout<<"err: "<<ITC_StsOutOfRange<<std::endl;

	ptr = itcDefaultAlloc( size, p_cvAllocUserData );
	if( !ptr )
		std::cout<<"err: "<<ITC_StsNoMem<<std::endl;

	//__END__;

	return ptr;
}

void  itcFree_( void* ptr )
{
	//CV_FUNCNAME( "cvFree_" );

	//__BEGIN__;

	if( ptr )
	{
		int status = itcDefaultFree( ptr, p_cvAllocUserData );
		if( status < 0 )
			std::cout<<"Deallocation error"<<std::endl;
	}

	//__END__;
}

/****************************************************************************************\
*            Functions for manipulating memory storage - list of memory blocks           *
\****************************************************************************************/

/* initializes allocated storage */
static void itcInitMemStorage( ItcMemStorage* storage, int block_size )
{
	//CV_FUNCNAME( "icvInitMemStorage " );

	//__BEGIN__;

	if( !storage )
		std::cout<<"err: "<<ITC_StsNullPtr<<std::endl;

	if( block_size <= 0 )
		block_size = ITC_STORAGE_BLOCK_SIZE;//block_size==0分配块大小默认为65408

	block_size = itcAlign( block_size, ITC_STRUCT_ALIGN );//待分配空间大小调整为8字节的倍数
	assert( sizeof(ItcMemBlock) % ITC_STRUCT_ALIGN == 0 );

	memset( storage, 0, sizeof( *storage ));
	storage->signature = ITC_STORAGE_MAGIC_VAL;
	storage->block_size = block_size;

	//__END__;
}

/* creates root memory storage */
/*
	为ItcMemStorage申请内存，block_size为大小，0为默认值，默认大小为65408
*/
ItcMemStorage* itcCreateMemStorage(int block_size)//当block_size==0则分配其大小为默认值
{
	ItcMemStorage *storage = 0;

	storage=(ItcMemStorage *) itcAlloc(sizeof(ItcMemStorage));

	itcInitMemStorage( storage, block_size );

	return storage;
}

/* creates child memory storage */
/*
	从父storage处获取子内存块
*/
ItcMemStorage* itcCreateChildMemStorage( ItcMemStorage * parent )
{
	ItcMemStorage *storage = 0;
	//CV_FUNCNAME( "cvCreateChildMemStorage" );

	//__BEGIN__;

	if (!parent)
		ITC_ERROR_(ITC_StsNullPtr);
		//std::cout<<"err: "<<ITC_StsNullPtr<<std::endl;

	//CV_CALL( storage = cvCreateMemStorage(parent->block_size));
	storage = itcCreateMemStorage(parent->block_size);
	storage->parent = parent;

	//__END__;

	//if( cvGetErrStatus() < 0 )
		//cvFree( &storage );

	return storage;
}

/* releases all blocks of the storage (or returns them to parent if any) */
static void itcDestroyMemStorage( ItcMemStorage* storage )
{
	//CV_FUNCNAME( "icvDestroyMemStorage" );

	//__BEGIN__;

	int k = 0;

	ItcMemBlock *block;
	ItcMemBlock *dst_top = 0;

	if (!storage)
		ITC_ERROR_(ITC_StsNullPtr);
		//std::cout<<"err: "<<ITC_StsNullPtr<<std::endl;

	if( storage->parent )
		dst_top = storage->parent->top;

	for( block = storage->bottom; block != 0; k++ )
	{
		ItcMemBlock *temp = block;

		block = block->next;
		if( storage->parent )
		{
			if( dst_top )
			{
				temp->prev = dst_top;
				temp->next = dst_top->next;
				if( temp->next )
					temp->next->prev = temp;
				dst_top = dst_top->next = temp;
			}
			else
			{
				dst_top = storage->parent->bottom = storage->parent->top = temp;
				temp->prev = temp->next = 0;
				storage->free_space = storage->block_size - sizeof( *temp );//释放storage中的空间还给parent
			}
		}
		//如果没有parent则逐个释放掉storage中每个block的内存
		else
		{
			//cvFree( &temp );
			itcFree_(&temp);
		}
	}

	storage->top = storage->bottom = 0;
	storage->free_space = 0;

	//__END__;
}

/* releases memory storage */
/*
	释放storage的所有内存
*/
void
	itcReleaseMemStorage( ItcMemStorage** storage )
{
	ItcMemStorage *st;
	//CV_FUNCNAME( "cvReleaseMemStorage" );

	//__BEGIN__;

	if( !storage )
		ITC_ERROR_(ITC_StsNullPtr);
		//std::cout<<"err: "<<ITC_StsNullPtr<<std::endl;

	//为什么不直接释放内存？
	st = *storage;
	*storage = 0;

	if( st )
	{
		itcDestroyMemStorage(st);
		itcFree_(&st);
		//CV_CALL( icvDestroyMemStorage( st ));
		//cvFree( &st );
	}

	//__END__;
}

/* clears memory storage (returns blocks to the parent if any) */
/*
	清空storage的内存，假如从父块继承的内存则还给父块
*/
void
	itcClearMemStorage( ItcMemStorage * storage )
{
	//CV_FUNCNAME( "cvClearMemStorage" );

	//__BEGIN__;

	if( !storage )
		ITC_ERROR_(ITC_StsNullPtr);
		//std::cout<<"err: "<<ITC_StsNullPtr<<std::endl;

	if( storage->parent )
	{
		itcDestroyMemStorage( storage );//假如有parent则把空间还给parent
	}
	else
	{
		//没有parent则清空该storage，只是清空，并不释放内存
		storage->top = storage->bottom;
		storage->free_space = storage->bottom ? storage->block_size - sizeof(ItcMemBlock) : 0;
	}

	//__END__;
}

/* moves stack pointer to next block.
   If no blocks, allocate new one and link it to the storage */
static void
itcGoNextMemBlock( ItcMemStorage * storage )
{
    //CV_FUNCNAME( "icvGoNextMemBlock" );
    
    //__BEGIN__;
    
	if (!storage)
		ITC_ERROR_(ITC_StsNullPtr);
        //std::cout<<"err: "<<ITC_StsNullPtr<<std::endl;

    if( !storage->top || !storage->top->next )
    {
        ItcMemBlock *block;

        if( !(storage->parent) )
        {
            //CV_CALL( block = (CvMemBlock *)cvAlloc( storage->block_size ));
			block = (ItcMemBlock *)itcAlloc( storage->block_size );
        }
        else
        {
            ItcMemStorage *parent = storage->parent;
            ItcMemStoragePos parent_pos;

            itcSaveMemStoragePos( parent, &parent_pos );
			itcGoNextMemBlock(parent);
            //CV_CALL( icvGoNextMemBlock( parent ));

            block = parent->top;
            itcRestoreMemStoragePos( parent, &parent_pos );

            if( block == parent->top )  /* the single allocated block */
            {
                assert( parent->bottom == block );
                parent->top = parent->bottom = 0;
                parent->free_space = 0;
            }
            else
            {
                /* cut the block from the parent's list of blocks */
                parent->top->next = block->next;
                if( block->next )
                    block->next->prev = parent->top;
            }
        }

        /* link block */
        block->next = 0;
        block->prev = storage->top;

        if( storage->top )
            storage->top->next = block;
        else
            storage->top = storage->bottom = block;
    }

    if( storage->top->next )
        storage->top = storage->top->next;
    storage->free_space = storage->block_size - sizeof(ItcMemBlock);
    assert( storage->free_space % ITC_STRUCT_ALIGN == 0 );

    //__END__;
}

/* remembers memory storage position */
void
	itcSaveMemStoragePos( const ItcMemStorage * storage, ItcMemStoragePos * pos )
{
	//CV_FUNCNAME( "cvSaveMemStoragePos" );

	//__BEGIN__;

	if( !storage || !pos )
		ITC_ERROR_(ITC_StsNullPtr);
		//std::cout<<"err: "<<ITC_StsNullPtr<<std::endl;

	pos->top = storage->top;
	pos->free_space = storage->free_space;

	//__END__;
}

/* restores memory storage position */
void
itcRestoreMemStoragePos( ItcMemStorage * storage, ItcMemStoragePos * pos )
{
   // CV_FUNCNAME( "cvRestoreMemStoragePos" );

    //__BEGIN__;

    if( !storage || !pos )
		ITC_ERROR_(ITC_StsNullPtr);
        //std::cout<<"err: "<<ITC_StsNullPtr<<std::endl;
    if( pos->free_space > storage->block_size )
		ITC_ERROR_(ITC_StsBadSize);
		//std::cout<<"err: "<<ITC_StsBadSize<<std::endl;
        //CV_ERROR( CV_StsBadSize, "" );

    /*
    // this breaks icvGoNextMemBlock, so comment it off for now
    if( storage->parent && (!pos->top || pos->top->next) )
    {
        CvMemBlock* save_bottom;
        if( !pos->top )
            save_bottom = 0;
        else
        {
            save_bottom = storage->bottom;
            storage->bottom = pos->top->next;
            pos->top->next = 0;
            storage->bottom->prev = 0;
        }
        icvDestroyMemStorage( storage );
        storage->bottom = save_bottom;
    }*/

    storage->top = pos->top;
    storage->free_space = pos->free_space;

    if( !storage->top )
    {
        storage->top = storage->bottom;
        storage->free_space = storage->top ? storage->block_size - sizeof(ItcMemBlock) : 0;
    }

    //__END__;
}

/* Allocates continuous buffer of the specified size in the storage */
void*
	itcMemStorageAlloc( ItcMemStorage* storage, size_t size )
{
	char *ptr = 0;

	//CV_FUNCNAME( "cvMemStorageAlloc" );

	//__BEGIN__;

	if( !storage )
		ITC_ERROR_(ITC_StsNullPtr);
		//CV_ERROR( CV_StsNullPtr, "NULL storage pointer" );
		//std::cout<<"err: "<<ITC_StsNullPtr<<" Null storage pointer"<<std::endl;

	if( size > INT_MAX )
		ITC_ERROR_(ITC_StsOutOfRange);
		//CV_ERROR( CV_StsOutOfRange, "Too large memory block is requested" );
		//std::cout<<"err: "<<ITC_StsOutOfRange<<" Too large memory block is requested"<<std::endl;

	assert( storage->free_space % ITC_STRUCT_ALIGN == 0 );

	if( (size_t)storage->free_space < size )
	{
		size_t max_free_space = itcAlignLeft(storage->block_size - sizeof(ItcMemBlock), ITC_STRUCT_ALIGN);
		if( max_free_space < size )
			//CV_ERROR( CV_StsOutOfRange, "requested size is negative or too big" );
			std::cout<<"err: "<<ITC_StsOutOfRange<<" Too large memory block is requested"<<std::endl;

		//CV_CALL( icvGoNextMemBlock( storage ));
		itcGoNextMemBlock(storage);
	}

	ptr = ITC_FREE_PTR(storage);
	assert( (size_t)ptr % ITC_STRUCT_ALIGN == 0 );
	storage->free_space = itcAlignLeft(storage->free_space - (int)size, ITC_STRUCT_ALIGN );

	//__END__;

	return ptr;
}

/****************************************************************************************\
*                               Sequence implementation                                  *
\****************************************************************************************/

/* creates empty sequence */
/*
	seq_flags 为序列中元素的类型
	header_size 为头的大小，必须大于等于ItcSeq的大小
	elem_size 为序列中元素的大小
	storage 为序列的内存空间

	ItcSeq *runs = itcCreateSeq(ITC_SEQ_ELTYPE_POINT, sizeof(ItcSeq), sizeof(ItcPoint), storage);
*/
ItcSeq *
	itcCreateSeq( int seq_flags, int header_size, int elem_size, ItcMemStorage * storage )
{
	ItcSeq *seq = 0;

	//CV_FUNCNAME( "cvCreateSeq" );

	//__BEGIN__;

	if (!storage)
		ITC_ERROR_(ITC_StsNullPtr);
		//std::cout<<"err: "<<ITC_StsNullPtr<<std::endl;
	if( header_size < (int)sizeof( ItcSeq ) || elem_size <= 0 )
		ITC_ERROR_(ITC_StsBadSize);
		//std::cout<<"err: "<<ITC_StsBadSize<<std::endl;

	/* allocate sequence header */
	//CV_CALL( seq = (CvSeq*)cvMemStorageAlloc( storage, header_size ));
	seq=(ItcSeq*)itcMemStorageAlloc(storage,header_size);
	memset( seq, 0, header_size );

	seq->header_size = header_size;
	seq->flags = (seq_flags & ~ITC_MAGIC_MASK) | ITC_SEQ_MAGIC_VAL;
	{
		int elemtype = ITC_MAT_TYPE(seq_flags);
		int typesize = ITC_ELEM_SIZE(elemtype);

		if (elemtype != CV_SEQ_ELTYPE_GENERIC &&
			typesize != 0 && typesize != elem_size)
			ITC_ERROR_DETAIL(ITC_StsBadSize, "Specified element size doesn't match to the size of the specified element type \
			(try to use 0 for element type)");
			//std::cout<<"err: "<<ITC_StsBadSize<<"Specified element size doesn't match to the size of the specified element type (try to use 0 for element type)"<<std::endl;
			//CV_ERROR( CV_StsBadSize,
			//"Specified element size doesn't match to the size of the specified element type "
			//"(try to use 0 for element type)" );
	}
	seq->elem_size = elem_size;
	seq->storage = storage;

	//CV_CALL( cvSetSeqBlockSize( seq, (1 << 10)/elem_size ));
	itcSetSeqBlockSize(seq, (1 << 10)/elem_size );

	//__END__;

	return seq;
}

/* adjusts <delta_elems> field of sequence. It determines how much the sequence
   grows if there are no free space inside the sequence buffers */
void
itcSetSeqBlockSize( ItcSeq *seq, int delta_elements )
{
    int elem_size;
    int useful_block_size;

    //CV_FUNCNAME( "cvSetSeqBlockSize" );

    //__BEGIN__;
	
    if( !seq || !seq->storage )
        std::cout<<"err: "<<ITC_StsNullPtr<<std::endl;
    if( delta_elements < 0 )
        std::cout<<"err: "<<ITC_StsBadSize<<std::endl;

    useful_block_size = itcAlignLeft(seq->storage->block_size - sizeof(ItcMemBlock) -
                                    sizeof(ItcSeqBlock), ITC_STRUCT_ALIGN);
    elem_size = seq->elem_size;

    if( delta_elements == 0 )
    {
        delta_elements = (1 << 10) / elem_size;
        delta_elements = ITC_MAX( delta_elements, 1 );
    }
    if( delta_elements * elem_size > useful_block_size )
    {
        delta_elements = useful_block_size / elem_size;
        if( delta_elements == 0 )
			std::cout<<"err: "<<ITC_StsBadSize<<"Storage block size is too small to fit the sequence elements"<<std::endl;
            //CV_ERROR( CV_StsOutOfRange, "Storage block size is too small "
                                       // "to fit the sequence elements" );
    }

    seq->delta_elems = delta_elements;

    //__END__;
}

/* finds sequence element by its index */
/*
	*seq为目标序列
	index为该序列中想要获取的元素的索引

	ItcPoint *p = (ItcPoint*)itcGetSeqElem(runs, i);
*/
char*
	itcGetSeqElem( const ItcSeq *seq, int index )
{
	ItcSeqBlock *block;
	int count, total = seq->total;

	if( (unsigned)index >= (unsigned)total )
	{
		index += index < 0 ? total : 0;
		index -= index >= total ? total : 0;
		if( (unsigned)index >= (unsigned)total )
			return 0;
	}

	block = seq->first;
	if( index + index <= total )
	{
		while( index >= (count = block->count) )
		{
			block = block->next;
			index -= count;
		}
	}
	else
	{
		do
		{
			block = block->prev;
			total -= block->count;
		}
		while( index < total );
		index -= total;
	}

	return block->data + index * seq->elem_size;
}

/* calculates index of sequence element */
int
	itcSeqElemIdx( const ItcSeq* seq, const void* _element, ItcSeqBlock** _block )
{
	const char *element = (const char *)_element;
	int elem_size;
	int id = -1;
	ItcSeqBlock *first_block;
	ItcSeqBlock *block;

	//CV_FUNCNAME( "cvSeqElemIdx" );

	//__BEGIN__;

	if( !seq || !element )
		std::cout<<"err: "<<ITC_StsNullPtr<<std::endl;

	block = first_block = seq->first;
	elem_size = seq->elem_size;

	for( ;; )
	{
		if( (unsigned)(element - block->data) < (unsigned) (block->count * elem_size) )
		{
			if( _block )
				*_block = block;
			if( elem_size <= ITC_SHIFT_TAB_MAX && (id = itcPower2ShiftTab[elem_size - 1]) >= 0 )
				id = (int)((size_t)(element - block->data) >> id);
			else
				id = (int)((size_t)(element - block->data) / elem_size);
			id += block->start_index - seq->first->start_index;
			break;
		}
		block = block->next;
		if( block == first_block )
			break;
	}

	//__END__;

	return id;
}

/* pushes element to the sequence */
/*
	seq为目标序列
	*element为想要加入序列的元素

	ItcPoint pt1 = itcPoint(i, i);
	itcSeqPush(runs, &pt1);
*/
char*
	itcSeqPush( ItcSeq *seq, void *element )
{
	char *ptr = 0;
	size_t elem_size;

	//CV_FUNCNAME( "cvSeqPush" );

	//__BEGIN__;

	if( !seq )
		std::cout<<"err: "<<ITC_StsNullPtr<<std::endl;
		//CV_ERROR( CV_StsNullPtr, "" );

	elem_size = seq->elem_size;
	ptr = seq->ptr;

	if( ptr >= seq->block_max )
	{
		//CV_CALL( icvGrowSeq( seq, 0 ));
		itcGrowSeq( seq, 0 );
		ptr = seq->ptr;
		assert( ptr + elem_size <= seq->block_max /*&& ptr == seq->block_min */  );
	}

	if( element )
		ITC_MEMCPY_AUTO(ptr, element, elem_size);
	seq->first->prev->count++;
	seq->total++;
	seq->ptr = ptr + elem_size;

	//__END__;

	return ptr;
}

/* pops the last element out of the sequence */
/*
	seq 为目标序列
	*element 为想要弹出的元素类型的指针

	ItcPoint temp ;
	itcSeqPop(runs, &temp);
*/
void itcSeqPop(ItcSeq *seq, void *element)
{
	char *ptr;
	int elem_size;

	//CV_FUNCNAME("cvSeqPop");

	//__BEGIN__;

	if (!seq)
		std::cout << "err: " << ITC_StsNullPtr << std::endl;
		//CV_ERROR(ITC_StsNullPtr, "");
	if (seq->total <= 0)
		std::cout << "err: " << ITC_StsBadSize << std::endl;
		//CV_ERROR(ITC_StsBadSize, "");

	elem_size = seq->elem_size;
	seq->ptr = ptr = seq->ptr - elem_size;

	if (element)
		ITC_MEMCPY_AUTO(element, ptr, elem_size);
	seq->ptr = ptr;
	seq->total--;

	if (--(seq->first->prev->count) == 0)
	{
		itcFreeSeqBlock(seq, 0);
		assert(seq->ptr == seq->block_max);
	}

	//__END__;
}

/* the function allocates space for at least one more sequence element.
   if there are free sequence blocks (seq->free_blocks != 0),
   they are reused, otherwise the space is allocated in the storage */
static void
itcGrowSeq( ItcSeq *seq, int in_front_of )
{
    //CV_FUNCNAME( "icvGrowSeq" );

    //__BEGIN__;

    ItcSeqBlock *block;

    if( !seq )
		std::cout<<"err: "<<ITC_StsNullPtr<<std::endl;
        //CV_ERROR( CV_StsNullPtr, "" );
    block = seq->free_blocks;

    if( !block )
    {
        int elem_size = seq->elem_size;
        int delta_elems = seq->delta_elems;
        ItcMemStorage *storage = seq->storage;

        if( seq->total >= delta_elems*4 )
            itcSetSeqBlockSize( seq, delta_elems*2 );

        if( !storage )
			std::cout<<"err: "<<ITC_StsNullPtr<<std::endl;
            //CV_ERROR( CV_StsNullPtr, "The sequence has NULL storage pointer" );

        /* if there is a free space just after last allocated block
           and it's big enough then enlarge the last block
           (this can happen only if the new block is added to the end of sequence */
        if( (unsigned)(ITC_FREE_PTR(storage) - seq->block_max) < ITC_STRUCT_ALIGN &&
            storage->free_space >= seq->elem_size && !in_front_of )
        {
            int delta = storage->free_space / elem_size;

            delta = ITC_MIN( delta, delta_elems ) * elem_size;
            seq->block_max += delta;
            storage->free_space = itcAlignLeft((int)(((char*)storage->top + storage->block_size) -
                                              seq->block_max), ITC_STRUCT_ALIGN );
            //EXIT;
        }
        else
        {
            int delta = elem_size * delta_elems + (int)itcAlign(sizeof(ItcSeqBlock), ITC_STRUCT_ALIGN);

            /* try to allocate <delta_elements> elements */
            if( storage->free_space < delta )
            {
                int small_block_size = ITC_MAX(1, delta_elems/3)*elem_size +
                                       (int)itcAlign(sizeof(ItcSeqBlock), ITC_STRUCT_ALIGN);
                /* try to allocate smaller part */
                if( storage->free_space >= small_block_size + ITC_STRUCT_ALIGN )
                {
                    delta = (storage->free_space - (int)itcAlign(sizeof(ItcSeqBlock), ITC_STRUCT_ALIGN))/seq->elem_size;
                    delta = delta*seq->elem_size + (int)itcAlign(sizeof(ItcSeqBlock), ITC_STRUCT_ALIGN);
                }
                else
                {
                    //CV_CALL( icvGoNextMemBlock( storage ));
					itcGoNextMemBlock(storage);
                    assert( storage->free_space >= delta );
                }
            }

            //CV_CALL( block = (CvSeqBlock*)cvMemStorageAlloc( storage, delta ));
			block = (ItcSeqBlock*)itcMemStorageAlloc( storage, delta );
            block->data = (char*)itcAlignPtr( block + 1, ITC_STRUCT_ALIGN );
            block->count = delta - (int)itcAlign(sizeof(ItcSeqBlock), ITC_STRUCT_ALIGN);
            block->prev = block->next = 0;
        }
    }
    else
    {
        seq->free_blocks = block->next;
    }

    if( !(seq->first) )
    {
        seq->first = block;
        block->prev = block->next = block;
    }
    else
    {
        block->prev = seq->first->prev;
        block->next = seq->first;
        block->prev->next = block->next->prev = block;
    }

    /* for free blocks the <count> field means total number of bytes in the block.
       And for used blocks it means a current number of sequence
       elements in the block */
    assert( block->count % seq->elem_size == 0 && block->count > 0 );

    if( !in_front_of )
    {
        seq->ptr = block->data;
        seq->block_max = block->data + block->count;
        block->start_index = block == block->prev ? 0 :
            block->prev->start_index + block->prev->count;
    }
    else
    {
        int delta = block->count / seq->elem_size;
        block->data += block->count;

        if( block != block->prev )
        {
            assert( seq->first->start_index == 0 );
            seq->first = block;
        }
        else
        {
            seq->block_max = seq->ptr = block->data;
        }

        block->start_index = 0;

        for( ;; )
        {
            block->start_index += delta;
            block = block->next;
            if( block == seq->first )
                break;
        }
    }

    block->count = 0;

    //__END__;
}

/* recycles a sequence block for the further use */
static void itcFreeSeqBlock(ItcSeq *seq, int in_front_of)
{
	/*CV_FUNCNAME( "icvFreeSeqBlock" );*/

	//__BEGIN__;

	ItcSeqBlock *block = seq->first;

	assert((in_front_of ? block : block->prev)->count == 0);

	if (block == block->prev)  /* single block case */
	{
		block->count = (int)(seq->block_max - block->data) + block->start_index * seq->elem_size;
		block->data = seq->block_max - block->count;
		seq->first = 0;
		seq->ptr = seq->block_max = 0;
		seq->total = 0;
	}
	else
	{
		if (!in_front_of)
		{
			block = block->prev;
			assert(seq->ptr == block->data);

			block->count = (int)(seq->block_max - seq->ptr);
			seq->block_max = seq->ptr = block->prev->data +
				block->prev->count * seq->elem_size;
		}
		else
		{
			int delta = block->start_index;

			block->count = delta * seq->elem_size;
			block->data -= block->count;

			/* update start indices of sequence blocks */
			for (;;)
			{
				block->start_index -= delta;
				block = block->next;
				if (block == seq->first)
					break;
			}

			seq->first = block->next;
		}

		block->prev->next = block->next;
		block->next->prev = block->prev;
	}

	assert(block->count > 0 && block->count % seq->elem_size == 0);
	block->next = seq->free_blocks;
	seq->free_blocks = block;

	//__END__;
}

/* pushes element to the front of the sequence */
/*
	ItcPoint temp = itcPoint(101,102);
	itcSeqPushFront(runs, &temp);
*/
char* itcSeqPushFront(ItcSeq *seq, void *element)
{
	char* ptr = 0;
	int elem_size;
	ItcSeqBlock *block;

	//CV_FUNCNAME("cvSeqPushFront");

	//__BEGIN__;

	if (!seq)
		std::cout << "err: " << ITC_StsNullPtr << std::endl;
		//CV_ERROR(CV_StsNullPtr, "");

	elem_size = seq->elem_size;
	block = seq->first;

	if (!block || block->start_index == 0)
	{
		//CV_CALL(icvGrowSeq(seq, 1));
		itcGrowSeq(seq, 1);

		block = seq->first;
		assert(block->start_index > 0);
	}

	ptr = block->data -= elem_size;

	if (element)
		ITC_MEMCPY_AUTO(ptr, element, elem_size);
	block->count++;
	block->start_index--;
	seq->total++;

	//__END__;

	return ptr;
}

/* pulls out the first element of the sequence */
/*
	用法同itcSeqPop
*/
void itcSeqPopFront(ItcSeq *seq, void *element)
{
	int elem_size;
	ItcSeqBlock *block;

	//CV_FUNCNAME("cvSeqPopFront");

	//__BEGIN__;

	if (!seq)
		std::cout << "err: " << ITC_StsNullPtr << std::endl;
		//CV_ERROR(CV_StsNullPtr, "");
	if (seq->total <= 0)
		std::cout << "err: " << ITC_StsBadSize << std::endl;
		//CV_ERROR(CV_StsBadSize, "");

	elem_size = seq->elem_size;
	block = seq->first;

	if (element)
		ITC_MEMCPY_AUTO(element, block->data, elem_size);
	block->data += elem_size;
	block->start_index++;
	seq->total--;

	if (--(block->count) == 0)
	{
		itcFreeSeqBlock(seq, 1);
	}

	//__END__;
}

/* inserts new element in the middle of the sequence */
/*
	seq 为目标序列
	before_index 为要插入的位置，不可以超过序列的元素总数目
	*element 为插入元素的类型

*/
char* itcSeqInsert(ItcSeq *seq, int before_index, void *element)
{
	int elem_size;
	int block_size;
	ItcSeqBlock *block;
	int delta_index;
	int total;
	char* ret_ptr = 0;

	//CV_FUNCNAME("cvSeqInsert");

	//__BEGIN__;

	if (!seq)
		std::cout << "err: " << ITC_StsNullPtr << std::endl;
		//CV_ERROR(CV_StsNullPtr, "");

	total = seq->total;
	before_index += before_index < 0 ? total : 0;
	before_index -= before_index > total ? total : 0;

	if ((unsigned)before_index > (unsigned)total)
		std::cout << "err: " << ITC_StsOutOfRange <<"Invalid index"<< std::endl;
		//CV_ERROR(CV_StsOutOfRange, "");

	if (before_index == total)
	{
		//CV_CALL(ret_ptr = cvSeqPush(seq, element));
		ret_ptr = itcSeqPush(seq, element);
	}
	else if (before_index == 0)
	{
		//CV_CALL(ret_ptr = cvSeqPushFront(seq, element));
		ret_ptr = itcSeqPushFront(seq, element);
	}
	else
	{
		elem_size = seq->elem_size;

		if (before_index >= total >> 1)
		{
			char *ptr = seq->ptr + elem_size;

			if (ptr > seq->block_max)
			{
				//CV_CALL(icvGrowSeq(seq, 0));
				itcGrowSeq(seq, 0);

				ptr = seq->ptr + elem_size;
				assert(ptr <= seq->block_max);
			}

			delta_index = seq->first->start_index;
			block = seq->first->prev;
			block->count++;
			block_size = (int)(ptr - block->data);

			while (before_index < block->start_index - delta_index)
			{
				ItcSeqBlock *prev_block = block->prev;

				memmove(block->data + elem_size, block->data, block_size - elem_size);
				block_size = prev_block->count * elem_size;
				memcpy(block->data, prev_block->data + block_size - elem_size, elem_size);
				block = prev_block;

				/* check that we don't fall in the infinite loop */
				assert(block != seq->first->prev);
			}

			before_index = (before_index - block->start_index + delta_index) * elem_size;
			memmove(block->data + before_index + elem_size, block->data + before_index,
				block_size - before_index - elem_size);

			ret_ptr = block->data + before_index;

			if (element)
				memcpy(ret_ptr, element, elem_size);
			seq->ptr = ptr;
		}
		else
		{
			block = seq->first;

			if (block->start_index == 0)
			{
				//CV_CALL(icvGrowSeq(seq, 1));
				itcGrowSeq(seq, 1);

				block = seq->first;
			}

			delta_index = block->start_index;
			block->count++;
			block->start_index--;
			block->data -= elem_size;

			while (before_index > block->start_index - delta_index + block->count)
			{
				ItcSeqBlock *next_block = block->next;

				block_size = block->count * elem_size;
				memmove(block->data, block->data + elem_size, block_size - elem_size);
				memcpy(block->data + block_size - elem_size, next_block->data, elem_size);
				block = next_block;
				/* check that we don't fall in the infinite loop */
				assert(block != seq->first);
			}

			before_index = (before_index - block->start_index + delta_index) * elem_size;
			memmove(block->data, block->data + elem_size, before_index - elem_size);

			ret_ptr = block->data + before_index - elem_size;

			if (element)
				memcpy(ret_ptr, element, elem_size);
		}

		seq->total = total + 1;
	}

	//__END__;

	return ret_ptr;
}

/* removes element from the sequence */
/*
	用法参考itcSeqPop，index为要移除的元素索引
*/
void itcSeqRemove(ItcSeq *seq, int index)
{
	char *ptr;
	int elem_size;
	int block_size;
	ItcSeqBlock *block;
	int delta_index;
	int total, front = 0;

	//CV_FUNCNAME("cvSeqRemove");

	//__BEGIN__;

	if (!seq)
		std::cout << "err: " << ITC_StsNullPtr << std::endl;
		//CV_ERROR(CV_StsNullPtr, "");

	total = seq->total;

	index += index < 0 ? total : 0;
	index -= index >= total ? total : 0;

	if ((unsigned)index >= (unsigned)total)
		std::cout << "err: " << ITC_StsOutOfRange << "Invalid index" << std::endl;
		//CV_ERROR(CV_StsOutOfRange, "Invalid index");

	if (index == total - 1)
	{
		itcSeqPop(seq, 0);
	}
	else if (index == 0)
	{
		itcSeqPopFront(seq, 0);
	}
	else
	{
		block = seq->first;
		elem_size = seq->elem_size;
		delta_index = block->start_index;
		while (block->start_index - delta_index + block->count <= index)
			block = block->next;

		ptr = block->data + (index - block->start_index + delta_index) * elem_size;

		front = index < total >> 1;
		if (!front)
		{
			block_size = block->count * elem_size - (int)(ptr - block->data);

			while (block != seq->first->prev)  /* while not the last block */
			{
				ItcSeqBlock *next_block = block->next;

				memmove(ptr, ptr + elem_size, block_size - elem_size);
				memcpy(ptr + block_size - elem_size, next_block->data, elem_size);
				block = next_block;
				ptr = block->data;
				block_size = block->count * elem_size;
			}

			memmove(ptr, ptr + elem_size, block_size - elem_size);
			seq->ptr -= elem_size;
		}
		else
		{
			ptr += elem_size;
			block_size = (int)(ptr - block->data);

			while (block != seq->first)
			{
				ItcSeqBlock *prev_block = block->prev;

				memmove(block->data + elem_size, block->data, block_size - elem_size);
				block_size = prev_block->count * elem_size;
				memcpy(block->data, prev_block->data + block_size - elem_size, elem_size);
				block = prev_block;
			}

			memmove(block->data + elem_size, block->data, block_size - elem_size);
			block->data += elem_size;
			block->start_index++;
		}

		seq->total = total - 1;
		if (--block->count == 0)
			itcFreeSeqBlock(seq, front);
	}

	//__END__;
}

/* initializes sequence writer */
void itcStartAppendToSeq(ItcSeq *seq, ItcSeqWriter * writer)
{
	//CV_FUNCNAME("cvStartAppendToSeq");

	//__BEGIN__;

	if (!seq || !writer)
		ITC_ERROR_(ITC_StsNullPtr);
		//CV_ERROR(CV_StsNullPtr, "");

	memset(writer, 0, sizeof(*writer));
	writer->header_size = sizeof(ItcSeqWriter);

	writer->seq = seq;
	writer->block = seq->first ? seq->first->prev : 0;
	writer->ptr = seq->ptr;
	writer->block_max = seq->block_max;

	//__END__;
}

/* initializes sequence writer */
void itcStartWriteSeq(int seq_flags, int header_size,
int elem_size, ItcMemStorage * storage, ItcSeqWriter * writer)
{
	ItcSeq *seq = 0;

	//CV_FUNCNAME("cvStartWriteSeq");

	//__BEGIN__;

	if (!storage || !writer)
		ITC_ERROR_(ITC_StsNullPtr);
		//CV_ERROR(CV_StsNullPtr, "");

	//CV_CALL(seq = cvCreateSeq(seq_flags, header_size, elem_size, storage));
	seq = itcCreateSeq(seq_flags, header_size, elem_size, storage);
	itcStartAppendToSeq(seq, writer);

	//__END__;
}

/* updates sequence header */
void itcFlushSeqWriter(ItcSeqWriter * writer)
{
	ItcSeq *seq = 0;

	//CV_FUNCNAME("cvFlushSeqWriter");

	//__BEGIN__;

	if (!writer)
		ITC_ERROR_(ITC_StsNullPtr);
		//CV_ERROR(CV_StsNullPtr, "");

	seq = writer->seq;
	seq->ptr = writer->ptr;

	if (writer->block)
	{
		int total = 0;
		ItcSeqBlock *first_block = writer->seq->first;
		ItcSeqBlock *block = first_block;

		writer->block->count = (int)((writer->ptr - writer->block->data) / seq->elem_size);
		assert(writer->block->count > 0);

		do
		{
			total += block->count;
			block = block->next;
		} while (block != first_block);

		writer->seq->total = total;
	}

	//__END__;
}

/* calls icvFlushSeqWriter and finishes writing process */
ItcSeq * itcEndWriteSeq(ItcSeqWriter * writer)
{
	ItcSeq *seq = 0;

	//CV_FUNCNAME("cvEndWriteSeq");

	//__BEGIN__;

	if (!writer)
		ITC_ERROR_(ITC_StsNullPtr);
		//CV_ERROR(CV_StsNullPtr, "");

	//CV_CALL(cvFlushSeqWriter(writer));
	itcFlushSeqWriter(writer);
	seq = writer->seq;

	/* truncate the last block */
	if (writer->block && writer->seq->storage)
	{
		ItcMemStorage *storage = seq->storage;
		char *storage_block_max = (char *)storage->top + storage->block_size;

		assert(writer->block->count > 0);

		if ((unsigned)((storage_block_max - storage->free_space)
			- seq->block_max) < ITC_STRUCT_ALIGN)
		{
			storage->free_space = itcAlignLeft((int)(storage_block_max - seq->ptr), ITC_STRUCT_ALIGN);
			seq->block_max = seq->ptr;
		}
	}

	writer->ptr = 0;

	//__END__;

	return seq;
}

/* initializes sequence reader */
void itcStartReadSeq(const ItcSeq *seq, ItcSeqReader * reader, int reverse)
{
	ItcSeqBlock *first_block;
	ItcSeqBlock *last_block;

	//CV_FUNCNAME("cvStartReadSeq");

	if (reader)
	{
		reader->seq = 0;
		reader->block = 0;
		reader->ptr = reader->block_max = reader->block_min = 0;
	}

	//__BEGIN__;

	if (!seq || !reader)
		ITC_ERROR_(ITC_StsNullPtr);
		//CV_ERROR(CV_StsNullPtr, "");

	reader->header_size = sizeof(ItcSeqReader);
	reader->seq = (ItcSeq*)seq;

	first_block = seq->first;

	if (first_block)
	{
		last_block = first_block->prev;
		reader->ptr = first_block->data;
		reader->prev_elem = ITC_GET_LAST_ELEM(seq, last_block);
		reader->delta_index = seq->first->start_index;

		if (reverse)
		{
			char *temp = reader->ptr;

			reader->ptr = reader->prev_elem;
			reader->prev_elem = temp;

			reader->block = last_block;
		}
		else
		{
			reader->block = first_block;
		}

		reader->block_min = reader->block->data;
		reader->block_max = reader->block_min + reader->block->count * seq->elem_size;
	}
	else
	{
		reader->delta_index = 0;
		reader->block = 0;

		reader->ptr = reader->prev_elem = reader->block_min = reader->block_max = 0;
	}

	//__END__;
}

/* adds several elements to the end or in the beginning of sequence */
/*
	向序列中推入多个元素，*_element为首地址，count为个数，front为标识符0意为在序列头部添加，非0为在序列尾部添加

	itcSeqPushMulti(runs, &temp, 5, ITC_FRONT);
*/
void itcSeqPushMulti(ItcSeq *seq, void *_elements, int count, int front)
{
	char *elements = (char *)_elements;

	//CV_FUNCNAME("cvSeqPushMulti");

	//__BEGIN__;
	int elem_size;

	if (!seq)
		ITC_ERROR_(ITC_StsNullPtr);
		//CV_ERROR(CV_StsNullPtr, "NULL sequence pointer");
	if (count < 0)
		ITC_ERROR_(ITC_StsBadSize);
		//CV_ERROR(CV_StsBadSize, "number of removed elements is negative");

	elem_size = seq->elem_size;

	if (!front)
	{
		while (count > 0)
		{
			int delta = (int)((seq->block_max - seq->ptr) / elem_size);

			delta = ITC_MIN(delta, count);
			if (delta > 0)
			{
				seq->first->prev->count += delta;
				seq->total += delta;
				count -= delta;
				delta *= elem_size;
				if (elements)
				{
					memcpy(seq->ptr, elements, delta);
					elements += delta;
				}
				seq->ptr += delta;
			}

			if (count > 0)
				itcGrowSeq(seq, 0);
				//CV_CALL(icvGrowSeq(seq, 0));
		}
	}
	else
	{
		ItcSeqBlock* block = seq->first;

		while (count > 0)
		{
			int delta;

			if (!block || block->start_index == 0)
			{
				itcGrowSeq(seq, 1);
				//CV_CALL(icvGrowSeq(seq, 1));

				block = seq->first;
				assert(block->start_index > 0);
			}

			delta = ITC_MIN(block->start_index, count);
			count -= delta;
			block->start_index -= delta;
			block->count += delta;
			seq->total += delta;
			delta *= elem_size;
			block->data -= delta;

			if (elements)
				memcpy(block->data, elements + count*elem_size, delta);
		}
	}

	//__END__;
}

/* removes several elements from the end of sequence */
/*
	用法参考itcSeqPushMulti
*/
void itcSeqPopMulti(ItcSeq *seq, void *_elements, int count, int front)
{
	char *elements = (char *)_elements;

	//CV_FUNCNAME("cvSeqPopMulti");

	//__BEGIN__;

	if (!seq)
		ITC_ERROR_(ITC_StsNullPtr);
		//CV_ERROR(CV_StsNullPtr, "NULL sequence pointer");
	if (count < 0)
		ITC_ERROR_(ITC_StsBadSize);
		//CV_ERROR(CV_StsBadSize, "number of removed elements is negative");

	count = ITC_MIN(count, seq->total);

	if (!front)
	{
		if (elements)
			elements += count * seq->elem_size;

		while (count > 0)
		{
			int delta = seq->first->prev->count;

			delta = ITC_MIN(delta, count);
			assert(delta > 0);

			seq->first->prev->count -= delta;
			seq->total -= delta;
			count -= delta;
			delta *= seq->elem_size;
			seq->ptr -= delta;

			if (elements)
			{
				elements -= delta;
				memcpy(elements, seq->ptr, delta);
			}

			if (seq->first->prev->count == 0)
				itcFreeSeqBlock(seq, 0);
				//icvFreeSeqBlock(seq, 0);
		}
	}
	else
	{
		while (count > 0)
		{
			int delta = seq->first->count;

			delta = ITC_MIN(delta, count);
			assert(delta > 0);

			seq->first->count -= delta;
			seq->total -= delta;
			count -= delta;
			seq->first->start_index += delta;
			delta *= seq->elem_size;

			if (elements)
			{
				memcpy(elements, seq->first->data, delta);
				elements += delta;
			}

			seq->first->data += delta;
			if (seq->first->count == 0)
				itcFreeSeqBlock(seq, 1);
				//icvFreeSeqBlock(seq, 1);
		}
	}

	//__END__;
}

/* removes all elements from the sequence */
/*
	清空序列中的元素，但并不释放storage的内存。

	如果先清空了storage中的内存，但并不影响序列中的元素，不清楚原因，还在看
*/
void itcClearSeq(ItcSeq *seq)
{
	//CV_FUNCNAME("cvClearSeq");

	//__BEGIN__;

	if (!seq)
		ITC_ERROR_(ITC_StsNullPtr);
	itcSeqPopMulti(seq, 0, seq->total,0);

	//__END__;
}

/* changes the current reading block to the previous or to the next */
void itcChangeSeqBlock(void* _reader, int direction)
{
	//CV_FUNCNAME("cvChangeSeqBlock");

	//__BEGIN__;

	ItcSeqReader* reader = (ItcSeqReader*)_reader;

	if (!reader)
		ITC_ERROR_(ITC_StsNullPtr);
		//CV_ERROR(CV_StsNullPtr, "");

	if (direction > 0)
	{
		reader->block = reader->block->next;
		reader->ptr = reader->block->data;
	}
	else
	{
		reader->block = reader->block->prev;
		reader->ptr = ITC_GET_LAST_ELEM(reader->seq, reader->block);
	}
	reader->block_min = reader->block->data;
	reader->block_max = reader->block_min + reader->block->count * reader->seq->elem_size;

	//__END__;
}

//int
//itcSliceLength( CvSlice slice, const CvSeq* seq )
//{
//    int total = seq->total;
//    int length = slice.end_index - slice.start_index;
//    
//    if( length != 0 )
//    {
//        if( slice.start_index < 0 )
//            slice.start_index += total;
//        if( slice.end_index <= 0 )
//            slice.end_index += total;
//
//        length = slice.end_index - slice.start_index;
//    }
//
//    if( length < 0 )
//    {
//        length += total;
//        /*if( length < 0 )
//            length += total;*/
//    }
//    else if( length > total )
//        length = total;
//
//    return length;
//}