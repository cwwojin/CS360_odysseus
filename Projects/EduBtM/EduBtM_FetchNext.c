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
 * Module: EduBtM_FetchNext.c
 *
 * Description:
 *  Find the next ObjectID satisfying the given condition. The current ObjectID
 *  is specified by the 'current'.
 *
 * Exports:
 *  Four EduBtM_FetchNext(PageID*, KeyDesc*, KeyValue*, Four, BtreeCursor*, BtreeCursor*)
 */


#include <string.h>
#include "EduBtM_common.h"
#include "BfM.h"
#include "EduBtM_Internal.h"


/*@ Internal Function Prototypes */
Four edubtm_FetchNext(KeyDesc*, KeyValue*, Four, BtreeCursor*, BtreeCursor*);



/*@================================
 * EduBtM_FetchNext()
 *================================*/
/*
 * Function: Four EduBtM_FetchNext(PageID*, KeyDesc*, KeyValue*,
 *                              Four, BtreeCursor*, BtreeCursor*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Fetch the next ObjectID satisfying the given condition.
 * By the B+ tree structure modification resulted from the splitting or merging
 * the current cursor may point to the invalid position. So we should adjust
 * the B+ tree cursor before using the cursor.
 *
 * Returns:
 *  error code
 *    eBADPARAMETER_BTM
 *    eBADCURSOR
 *    some errors caused by function calls
 */
Four EduBtM_FetchNext(
    PageID                      *root,          /* IN root page's PageID */
    KeyDesc                     *kdesc,         /* IN key descriptor */
    KeyValue                    *kval,          /* IN key value of stop condition */
    Four                        compOp,         /* IN comparison operator of stop condition */
    BtreeCursor                 *current,       /* IN current B+ tree cursor */
    BtreeCursor                 *next)          /* OUT next B+ tree cursor */
{
	/* These local variables are used in the solution code. However, you don¡¯t have to use all these variables in your code, and you may also declare and use additional local variables if needed. */
    int							i;
    Four                        e;              /* error number */
    Four                        cmp;            /* comparison result */
    Two                         slotNo;         /* slot no. of a leaf page */
    Two                         oidArrayElemNo; /* element no. of the array of ObjectIDs */
    Two                         alignedKlen;    /* aligned length of key length */
    PageID                      overflow;       /* temporary PageID of an overflow page */
    Boolean                     found;          /* search result */
    ObjectID                    *oidArray;      /* array of ObjectIDs */
    BtreeLeaf                   *apage;         /* pointer to a buffer holding a leaf page */
    BtreeOverflow               *opage;         /* pointer to a buffer holding an overflow page */
    btm_LeafEntry               *entry;         /* pointer to a leaf entry */
    BtreeCursor                 tCursor;        /* a temporary Btree cursor */
  
    
    /*@ check parameter */
    if (root == NULL || kdesc == NULL || kval == NULL || current == NULL || next == NULL)
	ERR(eBADPARAMETER_BTM);
    
    /* Is the current cursor valid? */
    if (current->flag != CURSOR_ON && current->flag != CURSOR_EOS)
		ERR(eBADCURSOR);
    
    if (current->flag == CURSOR_EOS) return(eNOERROR);
    
    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }
	
	/* NEWCODE */
	//1. Call edubtm_FetchNext() to retrieve the next object, given the stop condition.
	next->flag = CURSOR_INVALID;
	e = edubtm_FetchNext(kdesc, kval, compOp, current, next);
	if (e < 0) ERR(e);
	/* ENDOFNEWCODE*/

    
    return(eNOERROR);
    
} /* EduBtM_FetchNext() */



/*@================================
 * edubtm_FetchNext()
 *================================*/
/*
 * Function: Four edubtm_FetchNext(KeyDesc*, KeyValue*, Four,
 *                              BtreeCursor*, BtreeCursor*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Get the next item. We assume that the current cursor is valid; that is.
 *  'current' rightly points to an existing ObjectID.
 *
 * Returns:
 *  Error code
 *    eBADCOMPOP_BTM
 *    some errors caused by function calls
 */
Four edubtm_FetchNext(
    KeyDesc  		*kdesc,		/* IN key descriptor */
    KeyValue 		*kval,		/* IN key value of stop condition */
    Four     		compOp,		/* IN comparison operator of stop condition */
    BtreeCursor 	*current,	/* IN current cursor */
    BtreeCursor 	*next)		/* OUT next cursor */
{
	/* These local variables are used in the solution code. However, you don¡¯t have to use all these variables in your code, and you may also declare and use additional local variables if needed. */
    Four 		e;		/* error number */
    Four 		cmp;		/* comparison result */
    Two 		alignedKlen;	/* aligned length of a key length */
    PageID 		leaf;		/* temporary PageID of a leaf page */
    PageID 		overflow;	/* temporary PageID of an overflow page */
    ObjectID 		*oidArray;	/* array of ObjectIDs */
    BtreeLeaf 		*apage;		/* pointer to a buffer holding a leaf page */
    BtreeOverflow 	*opage;		/* pointer to a buffer holding an overflow page */
    btm_LeafEntry 	*entry;		/* pointer to a leaf entry */   
	
	Two		idx;
    
    
    /* Error check whether using not supported functionality by EduBtM */
    int i;
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }
	
	/* NEWCODE */
	//1. get leaf page into buffer. set LEAF as the current leaf page.
	leaf = current->leaf;
	switch(compOp){
		case SM_EQ:
		case SM_LT:
		case SM_LE:
		case SM_EOF:
			idx = current->slotNo + 1;
			break;
		case SM_GT:
		case SM_GE:
		case SM_BOF:
			idx = current->slotNo - 1;
			break;
	}
	e = BfM_GetTrain((TrainID*) &leaf, (char**)&apage, PAGE_BUF);
	if(e < 0) ERR(e);
	//2. if last slot, get the NEXT leaf page. if first slot, get the PREV leaf page.
	if(idx >= apage->hdr.nSlots){
		if(apage->hdr.nextPage == -1){
			next->flag = CURSOR_EOS;
			e = BfM_FreeTrain((TrainID*) &leaf, PAGE_BUF);
			if(e < 0) ERR(e);
			return(eNOERROR);
		}
		e = BfM_FreeTrain((TrainID*) &leaf, PAGE_BUF);
		if(e < 0) ERR(e);
		MAKE_PAGEID(leaf, leaf.volNo, apage->hdr.nextPage);
		e = BfM_GetTrain((TrainID*) &leaf, (char**)&apage, PAGE_BUF);
		if(e < 0) ERR(e);
		idx = 0;
	}
	else if(idx < 0){
		if(apage->hdr.prevPage == -1){
			//printf("reached BOF.\n");
			next->flag = CURSOR_EOS;
			e = BfM_FreeTrain((TrainID*) &leaf, PAGE_BUF);
			if(e < 0) ERR(e);
			return(eNOERROR);
		}
		e = BfM_FreeTrain((TrainID*) &leaf, PAGE_BUF);
		if(e < 0) ERR(e);
		MAKE_PAGEID(leaf, leaf.volNo, apage->hdr.prevPage);
		e = BfM_GetTrain((TrainID*) &leaf, (char**)&apage, PAGE_BUF);
		if(e < 0) ERR(e);
		idx = apage->hdr.nSlots - 1;
	}
	
	//3. get the target leaf entry. it should be in current->slotNo + 1 or slot #0.
	entry = &apage->data[apage->slot[-idx]];
	cmp = edubtm_KeyCompare(kdesc, (KeyValue*) &entry->klen, kval);
	switch(compOp){			//IF the stop condition is NOT satisfied, then set the next cursor's flag to CURSOR_EOS.
		case SM_EQ:
			if(cmp != EQUAL){
				next->flag = CURSOR_EOS;
			}
			break;
		case SM_LT:
			if(cmp != LESS){
				next->flag = CURSOR_EOS;
			}
			break;
		case SM_LE:
			if(cmp != LESS && cmp != EQUAL){
				next->flag = CURSOR_EOS;
			}
			break;
		case SM_GT:
			if(cmp != GREATER){
				next->flag = CURSOR_EOS;
			}
			break;
		case SM_GE:
			if(cmp != GREATER && cmp != EQUAL){
				next->flag = CURSOR_EOS;
			}
			break;
		default:
			break;
	}
	if(next->flag != CURSOR_EOS){	//stop condition satisfied. return this object.
		next->flag = CURSOR_ON;
		next->leaf = leaf;
		next->slotNo = idx;
		memcpy(&next->key, &entry->klen, sizeof(Two) + entry->klen);
		alignedKlen = ALIGNED_LENGTH(entry->klen);
		memcpy(&next->oid, &entry->kval[alignedKlen], sizeof(ObjectID));
	}
	//4. free the buffer.
	e = BfM_FreeTrain((TrainID*) &leaf, PAGE_BUF);
	if(e < 0) ERR(e);
	/* ENDOFNEWCODE */

    
    return(eNOERROR);
    
} /* edubtm_FetchNext() */
