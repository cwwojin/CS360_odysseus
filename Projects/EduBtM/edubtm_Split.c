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
 * Module: edubtm_Split.c
 *
 * Description : 
 *  This file has three functions about 'split'.
 *  'edubtm_SplitInternal(...) and edubtm_SplitLeaf(...) insert the given item
 *  after spliting, and return 'ritem' which should be inserted into the
 *  parent page.
 *
 * Exports:
 *  Four edubtm_SplitInternal(ObjectID*, BtreeInternal*, Two, InternalItem*, InternalItem*)
 *  Four edubtm_SplitLeaf(ObjectID*, PageID*, BtreeLeaf*, Two, LeafItem*, InternalItem*)
 */


#include <string.h>
#include "EduBtM_common.h"
#include "BfM.h"
#include "EduBtM_Internal.h"



/*@================================
 * edubtm_SplitInternal()
 *================================*/
/*
 * Function: Four edubtm_SplitInternal(ObjectID*, BtreeInternal*,Two, InternalItem*, InternalItem*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  At first, the function edubtm_SplitInternal(...) allocates a new internal page
 *  and initialize it.  Secondly, all items in the given page and the given
 *  'item' are divided by halves and stored to the two pages.  By spliting,
 *  the new internal item should be inserted into their parent and the item will
 *  be returned by 'ritem'.
 *
 *  A temporary page is used because it is difficult to use the given page
 *  directly and the temporary page will be copied to the given page later.
 *
 * Returns:
 *  error code
 *    some errors caused by function calls
 *
 * Note:
 *  The caller should call BfM_SetDirty() for 'fpage'.
 */
Four edubtm_SplitInternal(
    ObjectID                    *catObjForFile,         /* IN catalog object of B+ tree file */
    BtreeInternal               *fpage,                 /* INOUT the page which will be splitted */
    Two                         high,                   /* IN slot No. for the given 'item' */
    InternalItem                *item,                  /* IN the item which will be inserted */
    InternalItem                *ritem)                 /* OUT the item which will be returned by spliting */
{
	/* These local variables are used in the solution code. However, you don¡¯t have to use all these variables in your code, and you may also declare and use additional local variables if needed. */
    Four                        e;                      /* error number */
    Two                         i;                      /* slot No. in the given page, fpage */
    Two                         j;                      /* slot No. in the splitted pages */
    Two                         k;                      /* slot No. in the new page */
    Two                         maxLoop;                /* # of max loops; # of slots in fpage + 1 */
    Four                        sum;                    /* the size of a filled area */
    Boolean                     flag=FALSE;             /* TRUE if 'item' become a member of fpage */
    PageID                      newPid;                 /* for a New Allocated Page */
    BtreeInternal               *npage;                 /* a page pointer for the new allocated page */
    Two                         fEntryOffset;           /* starting offset of an entry in fpage */
    Two                         nEntryOffset;           /* starting offset of an entry in npage */
    Two                         entryLen;               /* length of an entry */
    btm_InternalEntry           *fEntry;                /* internal entry in the given page, fpage */
    btm_InternalEntry           *nEntry;                /* internal entry in the new page, npage*/
    Boolean                     isTmp;
	BtreeInternal		tpage;			/*Temporary page.*/
	
	/* NEWCODE */
	//1. Allocate a new page, init as internal.
	e = btm_AllocPage(catObjForFile, &fpage->hdr.pid, &newPid);
	if(e < 0) ERR(e);
	e = edubtm_InitInternal(&newPid, FALSE, isTmp);
	if(e < 0) ERR(e);
	e = BfM_GetTrain((TrainID*)&newPid, (char**)&npage, PAGE_BUF);
	if(e < 0) ERR(e);
	//2. Save the entries (+ New ITEM) to the 2 pages, "fpage" & "npage".
	maxLoop = fpage->hdr.nSlots + 1;
	tpage = *fpage;		//save fpage to temporary page TPAGE.
	flag = (high + 1) <= maxLoop / 2;
	for(i=0; i<maxLoop; i++){
		if(i == (maxLoop)/2 + 1){	//RETURN value : ritem
			if(i == high + 1){	//save ITEM.
				memcpy(ritem, item, sizeof(InternalItem));
			}
			else{			//save tpage's slot# (i) or (i-1)
				if(i > high + 1){
					fEntryOffset = tpage.slot[-(i-1)];
					fEntry = &tpage.data[fEntryOffset];
				}
				else{
					fEntryOffset = tpage.slot[-i];
					fEntry = &tpage.data[fEntryOffset];
				}
				//fEntry -> ritem
				entryLen = sizeof(ShortPageID) + ALIGNED_LENGTH(sizeof(Two) + item->klen);
				memcpy(ritem, fEntry, entryLen);
			}
		}
		else if(i > (maxLoop)/2 + 1){	//new page : npage
			nEntryOffset = npage->slot[npage->hdr.free];
			nEntry = &npage->data[nEntryOffset];
			if(i == high + 1){	//save ITEM.
				memcpy(nEntry, item, sizeof(ShortPageID) + sizeof(Two) + item->klen);
				entryLen = sizeof(ShortPageID) + ALIGNED_LENGTH(sizeof(Two) + item->klen);
			}
			else{			//save tpage's slot# (i) or (i-1)
				if(i > high + 1){
					fEntryOffset = tpage.slot[-(i-1)];
					fEntry = &tpage.data[fEntryOffset];
				}
				else{
					fEntryOffset = tpage.slot[-i];
					fEntry = &tpage.data[fEntryOffset];
				}
				entryLen = sizeof(ShortPageID) + ALIGNED_LENGTH(sizeof(Two) + item->klen);
				memcpy(nEntry, fEntry, entryLen);
				if(fEntryOffset + entryLen == fpage->hdr.free){
					fpage->hdr.free -= entryLen;
				}
				else{
					fpage->hdr.unused += entryLen;
				}
				fpage->hdr.nSlots--;
			}
			npage->slot[-(npage->hdr.nSlots)] = npage->hdr.free;
			npage->hdr.free += entryLen;
			npage->hdr.nSlots++;
		}
		else{	//original page : fpage
			if(i > high + 1){	//adjust slot.
				fpage->slot[-i] = tpage.slot[-(i-1)];
			}
			//else : do NOTHING.
		}
	}
	if(flag){
		entryLen = sizeof(ShortPageID) + ALIGNED_LENGTH(sizeof(Two) + item->klen);
		if(entryLen + sizeof(Two) > BI_CFREE(fpage)){
			edubtm_CompactInternalPage(fpage, NIL);
		}
		fEntry = &fpage->data[fpage->hdr.free];
		memcpy(fEntry, item, sizeof(ShortPageID) + sizeof(Two) + item->klen);
		fpage->slot[-(high + 1)] = fpage->hdr.free;
		fpage->hdr.free += entryLen;
		fpage->hdr.nSlots++;
	}
	/* ENDOFNEWCODE */

    
    return(eNOERROR);
    
} /* edubtm_SplitInternal() */



/*@================================
 * edubtm_SplitLeaf()
 *================================*/
/*
 * Function: Four edubtm_SplitLeaf(ObjectID*, PageID*, BtreeLeaf*, Two, LeafItem*, InternalItem*)
 *
 * Description: 
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  The function edubtm_SplitLeaf(...) is similar to edubtm_SplitInternal(...) except
 *  that the entry of a leaf differs from the entry of an internal and the first
 *  key value of a new page is used to make an internal item of their parent.
 *  Internal pages do not maintain the linked list, but leaves do it, so links
 *  are properly updated.
 *
 * Returns:
 *  Error code
 *  eDUPLICATEDOBJECTID_BTM
 *    some errors caused by function calls
 *
 * Note:
 *  The caller should call BfM_SetDirty() for 'fpage'.
 */
Four edubtm_SplitLeaf(
    ObjectID                    *catObjForFile, /* IN catalog object of B+ tree file */
    PageID                      *root,          /* IN PageID for the given page, 'fpage' */
    BtreeLeaf                   *fpage,         /* INOUT the page which will be splitted */
    Two                         high,           /* IN slotNo for the given 'item' */
    LeafItem                    *item,          /* IN the item which will be inserted */
    InternalItem                *ritem)         /* OUT the item which will be returned by spliting */
{
	/* These local variables are used in the solution code. However, you don¡¯t have to use all these variables in your code, and you may also declare and use additional local variables if needed. */
    Four                        e;              /* error number */
    Two                         i;              /* slot No. in the given page, fpage */
    Two                         j;              /* slot No. in the splitted pages */
    Two                         k;              /* slot No. in the new page */
    Two                         maxLoop;        /* # of max loops; # of slots in fpage + 1 */
    Four                        sum;            /* the size of a filled area */
    PageID                      newPid;         /* for a New Allocated Page */
    PageID                      nextPid;        /* for maintaining doubly linked list */
    BtreeLeaf                   tpage;          /* a temporary page for the given page */
    BtreeLeaf                   *npage;         /* a page pointer for the new page */
    BtreeLeaf                   *mpage;         /* for doubly linked list */
    btm_LeafEntry               *itemEntry;     /* entry for the given 'item' */
    btm_LeafEntry               *fEntry;        /* an entry in the given page, 'fpage' */
    btm_LeafEntry               *nEntry;        /* an entry in the new page, 'npage' */
    ObjectID                    *iOidArray;     /* ObjectID array of 'itemEntry' */
    ObjectID                    *fOidArray;     /* ObjectID array of 'fEntry' */
    Two                         fEntryOffset;   /* starting offset of 'fEntry' */
    Two                         nEntryOffset;   /* starting offset of 'nEntry' */
    Two                         oidArrayNo;     /* element No in an ObjectID array */
    Two                         alignedKlen;    /* aligned length of the key length */
    Two                         itemEntryLen;   /* length of entry for item */
    Two                         entryLen;       /* entry length */
    Boolean                     flag;
    Boolean                     isTmp;
	
	/* NEWCODE */
	//1. allocate a new page.
	e = btm_AllocPage(catObjForFile, root, &newPid);
	if(e < 0) ERR(e);
	//2. initialize new page as LEAF & get buffer.
	e = edubtm_InitLeaf(&newPid, FALSE, isTmp);
	if(e < 0) ERR(e);
	e = BfM_GetTrain((TrainID*)&newPid, (char**)&npage, PAGE_BUF);
	if(e < 0) ERR(e);
	//3. save the entries (+ new litem) in the original & new pages.
	maxLoop = fpage->hdr.nSlots + 1;
	tpage = *fpage;		//save fpage to temporary page TPAGE.
	flag = (high + 1 > (maxLoop)/ 2);	//TRUE : saves ITEM to npage.
	for(i=0; i<maxLoop; i++){
		if(i > (maxLoop)/ 2){	//npage.
			if(i == high + 1){	//save ITEM.
				nEntry = &npage->data[npage->hdr.free];
				nEntry->nObjects = item->nObjects;
				memcpy(&nEntry->klen, &item->klen, sizeof(Two) + item->klen);
				alignedKlen = ALIGNED_LENGTH(item->klen);
				memcpy(&nEntry->kval[alignedKlen], item, sizeof(ObjectID));
				entryLen = sizeof(Two) + sizeof(Two) + alignedKlen + sizeof(ObjectID);
			}
			else{			//save tpage's slot# (i) or (i-1)
				nEntry = &npage->data[npage->hdr.free];
				if(i > high + 1){
					fEntryOffset = tpage.slot[-(i-1)];
					fEntry = &tpage.data[fEntryOffset];
				}
				else{
					fEntryOffset = tpage.slot[-i];
					fEntry = &tpage.data[fEntryOffset];
				}
				alignedKlen = ALIGNED_LENGTH(fEntry->klen);
				entryLen = sizeof(Two) + sizeof(Two) + alignedKlen + sizeof(ObjectID);
				memcpy(nEntry, fEntry, entryLen);
				//Remove this entry from FPAGE.
				if(fEntryOffset + entryLen == fpage->hdr.free){
					fpage->hdr.free -= entryLen;
				}
				else{
					fpage->hdr.unused += entryLen;
				}
				fpage->hdr.nSlots--;
			}
			npage->slot[-(npage->hdr.nSlots)] = npage->hdr.free;
			npage->hdr.free += entryLen;
			npage->hdr.nSlots++;
		}
		else{	//original page : fpage
			if(i > high + 1){	//adjust slot.
				fpage->slot[-i] = tpage.slot[-(i-1)];
			}
			//else : do NOTHING.
		}
	}
	if(!flag){	//should save ITEM to fpage.
		alignedKlen = ALIGNED_LENGTH(item->klen);
		entryLen = sizeof(Two) + sizeof(Two) + alignedKlen + sizeof(ObjectID);
		if(entryLen + sizeof(Two) > BL_CFREE(fpage)){
			edubtm_CompactLeafPage(fpage, NIL);
		}
		//printf("pageNo : %d, entryLen : %d, FREE : %d, UNUSED : %d\n",root->pageNo, entryLen, fpage->hdr.free, fpage->hdr.unused);
		fEntry = &fpage->data[fpage->hdr.free];
		fEntry->nObjects = item->nObjects;
		memcpy(&fEntry->klen, &item->klen, sizeof(Two) + item->klen);
		memcpy(&fEntry->kval[alignedKlen], item, sizeof(ObjectID));
		fpage->slot[-(high + 1)] = fpage->hdr.free;
		fpage->hdr.free += entryLen;
		fpage->hdr.nSlots++;
	}
	//4. Update headers & doubly linked list.
	if(fpage->hdr.nextPage != NIL){
		npage->hdr.nextPage = fpage->hdr.nextPage;
		MAKE_PAGEID(nextPid, root->volNo, fpage->hdr.nextPage);	//set NEXTPAGE's prevPage to NPAGE.
		e = BfM_GetTrain((TrainID*)&nextPid, (char**)&mpage, PAGE_BUF);
		if(e < 0) ERR(e);
		mpage->hdr.prevPage = newPid.pageNo;
		e = BfM_SetDirty((TrainID*)&nextPid, PAGE_BUF);
		if(e < 0) ERRB1(e, &nextPid, PAGE_BUF);
		e = BfM_FreeTrain((TrainID*)&nextPid, PAGE_BUF);
		if(e < 0) ERR(e);	
	}
	fpage->hdr.nextPage = newPid.pageNo;
	npage->hdr.prevPage = root->pageNo;
	//5. Make the discriminator IEntry -> slot# 0. of NPAGE.
	nEntry = &npage->data[npage->slot[0]];
	ritem->spid = newPid.pageNo;
	memcpy(&ritem->klen, &nEntry->klen, sizeof(Two) + nEntry->klen);
	//6. Set dirty & free.
	e = BfM_SetDirty((TrainID*)&newPid, PAGE_BUF);
	if(e < 0) ERRB1(e, &newPid, PAGE_BUF);
	e = BfM_FreeTrain((TrainID*)&newPid, PAGE_BUF);
	if(e < 0) ERR(e);
	/* ENDOFNEWCODE */
 
    

    return(eNOERROR);
    
} /* edubtm_SplitLeaf() */
