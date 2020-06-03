/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2013-2015, Kyu-Young Whang, KAIST                         */
/*    All rights reserved.                                                    */
/*                                                                            */
/*    Redistribution and use in source and binary forms, with or without      */
/*    modification, are permitted provided that the following conditions      */
/*    are met:                                                                */
/*                                                                            */
/*    1. Redistributions of source code must retain the above copyright       */
/*       notice, this list of conditions and the following disclaimer.        */
/*                                                                            */
/*    2. Redistributions in binary form must reproduce the above copyright    */
/*       notice, this list of conditions and the following disclaimer in      */
/*       the documentation and/or other materials provided with the           */
/*       distribution.                                                        */
/*                                                                            */
/*    3. Neither the name of the copyright holder nor the names of its        */
/*       contributors may be used to endorse or promote products derived      */
/*       from this software without specific prior written permission.        */
/*                                                                            */
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS     */
/*    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT       */
/*    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS       */
/*    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE          */
/*    COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,    */
/*    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;        */
/*    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER        */
/*    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT      */
/*    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN       */
/*    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE         */
/*    POSSIBILITY OF SUCH DAMAGE.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/*    ODYSSEUS/EduCOSMOS Educational Purpose Object Storage System            */
/*    (Version 1.0)                                                           */
/*                                                                            */
/*    Developed by Professor Kyu-Young Whang et al.                           */
/*                                                                            */
/*    Advanced Information Technology Research Center (AITrc)                 */
/*    Korea Advanced Institute of Science and Technology (KAIST)              */
/*                                                                            */
/*    e-mail: odysseus.educosmos@gmail.com                                    */
/*                                                                            */
/******************************************************************************/
/*
 * Module: edubtm_root.c
 *
 * Description : 
 *  This file has two routines which are concerned with the changing the
 *  current root node. When an overflow or a underflow occurs in the root page
 *  the root node should be changed. But we don't change the root node to
 *  the new page. The old root page is used as the new root node; thus the
 *  root page is fixed always.
 *
 * Exports:
 *  Four edubtm_root_insert(ObjectID*, PageID*, InternalItem*)
 */


#include <string.h>
#include "EduBtM_common.h"
#include "Util.h"
#include "BfM.h"
#include "EduBtM_Internal.h"



/*@================================
 * edubtm_root_insert()
 *================================*/
/*
 * Function: Four edubtm_root_insert(ObjectID*, PageID*, InternalItem*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  This routine is called when a new entry was inserted into the root page and
 *  it was splitted two pages, 'root' and 'item->pid'. The new root should be
 *  made by the given root Page IDentifier and the sibling entry 'item'.
 *  We make it a rule to fix the root page; so a new page is allocated and
 *  the root node is copied into the newly allocated page. The root node
 *  is changed so that it points to the newly allocated node and the 'item->pid'.
 *
 * Returns:
 *  Error code
 *    some errors caused by function calls
 */
Four edubtm_root_insert(
    ObjectID     *catObjForFile, /* IN catalog object of B+ tree file */
    PageID       *root,		 /* IN root Page IDentifier */
    InternalItem *item)		 /* IN Internal item which will be the unique entry of the new root */
{
	/* These local variables are used in the solution code. However, you don¡¯t have to use all these variables in your code, and you may also declare and use additional local variables if needed. */
    Four      e;		/* error number */
    PageID    newPid;		/* newly allocated page */
    PageID    nextPid;		/* PageID of the next page of root if root is leaf */
    BtreePage *rootPage;	/* pointer to a buffer holding the root page */
    BtreePage *newPage;		/* pointer to a buffer holding the new page */
    BtreeLeaf *nextPage;	/* pointer to a buffer holding next page of root */
    btm_InternalEntry *entry;	/* an internal entry */
    Boolean   isTmp;
	
	/* NEWCODE */
	//1. Get the root page.
	e = BfM_GetTrain((TrainID*)root, (char**)&rootPage, PAGE_BUF);
	if(e < 0) ERR(e);
	//2. Allocate new page & get buffer.
	e = btm_AllocPage(catObjForFile, (PageID*)root, &newPid);
	if(e < 0) ERR(e);
	e = BfM_GetNewTrain((TrainID*)&newPid, (char**)&newPage, PAGE_BUF);
	if(e < 0) ERR(e);
	printf("newPid : (%d, %d)\n", newPid.volNo, newPid.pageNo);
	//3. Copy original root to the new page.
	memcpy(newPage, rootPage, PAGESIZE);
	newPage->any.hdr.pid = newPid;
	newPage->any.hdr.type = newPage->any.hdr.type & ~(ROOT);	//turn OFF ROOT!!
	//4. Initialize original root as the new root.
	e = edubtm_InitInternal(root, TRUE, isTmp);
	if(e < 0) ERR(e);
	//5. Insert ITEM into the root.
	Two alignedKlen = ALIGNED_LENGTH(sizeof(Two) + item->klen);
	Two entryLen = sizeof(ShortPageID) + alignedKlen;
	entry = &rootPage->bi.data[rootPage->bi.hdr.free];
	memcpy(entry, item, sizeof(ShortPageID) + sizeof(Two) + item->klen);	//spid, klen, kval(unaligned)
	rootPage->bi.slot[-(rootPage->bi.hdr.nSlots)] = rootPage->bi.hdr.free;
	rootPage->bi.hdr.free += entryLen;
	rootPage->bi.hdr.nSlots++;
	rootPage->bi.hdr.p0 = newPid.pageNo;	//p0 links to the NEW page.
	//7. IF both children are Leaves (newPage is Leaf) -> set doubly-linked list.
	printf("newPage type : %d, LEAF : %d, type&LEAF : %d\n", newPage->any.hdr.type, LEAF, newPage->any.hdr.type & LEAF);
	if(newPage->any.hdr.type & LEAF == LEAF){
		MAKE_PAGEID(nextPid, root->volNo, newPage->bl.hdr.nextPage);
		e = BfM_GetTrain((TrainID*)&nextPid, (char**)&nextPage, PAGE_BUF);
		if(e < 0) ERR(e);
		nextPage->hdr.prevPage = newPid.pageNo;
		printf("newPage points to : %d, nextPage points back to : %d\n", newPage->bl.hdr.nextPage, nextPage->hdr.prevPage);
		e = BfM_SetDirty((TrainID*)&nextPid, PAGE_BUF);
		if(e < 0) ERRB1(e, &nextPid, PAGE_BUF);
		e = BfM_FreeTrain((TrainID*)&nextPid, PAGE_BUF);
		if(e < 0) ERR(e);
	}
	printf("newPage points to : %d\n", newPage->bl.hdr.nextPage);
	//8. Set Dirty & Free -> root & newPage.
	e = BfM_SetDirty((TrainID*)&newPid, PAGE_BUF);
	if(e < 0) ERRB1(e, &newPid, PAGE_BUF);
	e = BfM_FreeTrain((TrainID*)&newPid, PAGE_BUF);
	if(e < 0) ERR(e);
	e = BfM_SetDirty((TrainID*)root, PAGE_BUF);
	if(e < 0) ERRB1(e, &newPid, PAGE_BUF);
	e = BfM_FreeTrain((TrainID*)root, PAGE_BUF);
	if(e < 0) ERR(e);
	/* ENDOFNEWCODE */
    
    return(eNOERROR);
    
} /* edubtm_root_insert() */
