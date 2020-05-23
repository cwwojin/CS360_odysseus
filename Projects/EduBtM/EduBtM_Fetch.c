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
 * Module: EduBtM_Fetch.c
 *
 * Description :
 *  Find the first object satisfying the given condition.
 *  If there is no such object, then return with 'flag' field of cursor set
 *  to CURSOR_EOS. If there is an object satisfying the condition, then cursor
 *  points to the object position in the B+ tree and the object identifier
 *  is returned via 'cursor' parameter.
 *  The condition is given with a key value and a comparison operator;
 *  the comparison operator is one among SM_BOF, SM_EOF, SM_EQ, SM_LT, SM_LE, SM_GT, SM_GE.
 *
 * Exports:
 *  Four EduBtM_Fetch(PageID*, KeyDesc*, KeyValue*, Four, KeyValue*, Four, BtreeCursor*)
 */


#include <string.h>
#include "EduBtM_common.h"
#include "BfM.h"
#include "EduBtM_Internal.h"


/*@ Internal Function Prototypes */
Four edubtm_Fetch(PageID*, KeyDesc*, KeyValue*, Four, KeyValue*, Four, BtreeCursor*);



/*@================================
 * EduBtM_Fetch()
 *================================*/
/*
 * Function: Four EduBtM_Fetch(PageID*, KeyDesc*, KeyVlaue*, Four, KeyValue*, Four, BtreeCursor*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Find the first object satisfying the given condition. See above for detail.
 *
 * Returns:
 *  error code
 *    eBADPARAMETER_BTM
 *    some errors caused by function calls
 *
 * Side effects:
 *  cursor  : The found ObjectID and its position in the Btree Leaf
 *            (it may indicate a ObjectID in an  overflow page).
 */
Four EduBtM_Fetch(
    PageID   *root,		/* IN The current root of the subtree */
    KeyDesc  *kdesc,		/* IN Btree key descriptor */
    KeyValue *startKval,	/* IN key value of start condition */
    Four     startCompOp,	/* IN comparison operator of start condition */
    KeyValue *stopKval,		/* IN key value of stop condition */
    Four     stopCompOp,	/* IN comparison operator of stop condition */
    BtreeCursor *cursor)	/* OUT Btree Cursor */
{
	/* These local variables are used in the solution code. However, you don¡¯t have to use all these variables in your code, and you may also declare and use additional local variables if needed. */
    int i;
    Four e;		   /* error number */

    
    if (root == NULL) ERR(eBADPARAMETER_BTM);

    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }
	
	/* NEWCODE */
	//Turn ON the cursor.
	cursor->flag = CURSOR_INVALID;
	//1. cases depending on startCompOp value.
	switch(startCompOp){
		case SM_BOF :
			//get the first object -> edubtm_FirstObject().
			e = btm_FirstObject(root, kdesc, stopKval, stopCompOp, cursor);
			if (e < 0) ERR(e);
			break;
		case SM_EOF :
			//get the last object -> edubtm_LastObject().
			e = btm_LastObject(root, kdesc, stopKval, stopCompOp, cursor);
			if (e < 0) ERR(e);
			break;
		default :
			//call edubtm_Fetch().
			e = edubtm_Fetch(root, kdesc, startKval, startCompOp, stopKval, stopCompOp, cursor);
			//e = btm_Fetch(root, kdesc, startKval, startCompOp, stopKval, stopCompOp, cursor);
			if (e < 0) ERR(e);
			break;
	}
	/* ENDOFNEWCODE */
    

    return(eNOERROR);

} /* EduBtM_Fetch() */



/*@================================
 * edubtm_Fetch()
 *================================*/
/*
 * Function: Four edubtm_Fetch(PageID*, KeyDesc*, KeyVlaue*, Four, KeyValue*, Four, BtreeCursor*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Find the first object satisfying the given condition.
 *  This function handles only the following conditions:
 *  SM_EQ, SM_LT, SM_LE, SM_GT, SM_GE.
 *
 * Returns:
 *  Error code *   
 *    eBADCOMPOP_BTM
 *    eBADBTREEPAGE_BTM
 *    some errors caused by function calls
 */
Four edubtm_Fetch(
    PageID              *root,          /* IN The current root of the subtree */
    KeyDesc             *kdesc,         /* IN Btree key descriptor */
    KeyValue            *startKval,     /* IN key value of start condition */
    Four                startCompOp,    /* IN comparison operator of start condition */
    KeyValue            *stopKval,      /* IN key value of stop condition */
    Four                stopCompOp,     /* IN comparison operator of stop condition */
    BtreeCursor         *cursor)        /* OUT Btree Cursor */
{
	/* These local variables are used in the solution code. However, you don¡¯t have to use all these variables in your code, and you may also declare and use additional local variables if needed. */
    Four                e;              /* error number */
    Four                cmp;            /* result of comparison */
    Two                 idx;            /* index */
    PageID              child;          /* child page when the root is an internla page */
    Two                 alignedKlen;    /* aligned size of the key length */
    BtreePage           *apage;         /* a Page Pointer to the given root */
    BtreeOverflow       *opage;         /* a page pointer if it necessary to access an overflow page */
    Boolean             found;          /* search result */
    PageID              *leafPid;       /* leaf page pointed by the cursor */
    Two                 slotNo;         /* slot pointed by the slot */
    PageID              ovPid;          /* PageID of the overflow page */
    PageNo              ovPageNo;       /* PageNo of the overflow page */
    PageID              prevPid;        /* PageID of the previous page */
    PageID              nextPid;        /* PageID of the next page */
    ObjectID            *oidArray;      /* array of the ObjectIDs */
    Two                 iEntryOffset;   /* starting offset of an internal entry */
    btm_InternalEntry   *iEntry;        /* an internal entry */
    Two                 lEntryOffset;   /* starting offset of a leaf entry */
    btm_LeafEntry       *lEntry;        /* a leaf entry */


    /* Error check whether using not supported functionality by EduBtM */
    int i;
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }
	
	/* NEWCODE */
	//1. get the root.
	e = BfM_GetTrain((TrainID*) root, (char**)&apage, PAGE_BUF);
	if(e < 0) ERR(e);
	//2. check if root is internal or leaf. apage->any.hdr.type
	if((apage->any.hdr.type & INTERNAL) == INTERNAL){
		found = edubtm_BinarySearchInternal(apage, kdesc, startKval, &idx);	//get the slot#. of the target entry.
		if(idx == -1){
			MAKE_PAGEID(child, root->volNo, apage->bi.hdr.p0);		//NEXT child to visit.
		}
		else{
			iEntryOffset = apage->bi.slot[-idx];
			iEntry = &apage->bi.data[iEntryOffset];
			MAKE_PAGEID(child, root->volNo, iEntry->spid);		//NEXT child to visit.
		}
		e = BfM_FreeTrain((TrainID*) root, PAGE_BUF);		//CAN free buffer here.
		if(e < 0) ERR(e);
		e = edubtm_Fetch(&child, kdesc, startKval, startCompOp, stopKval, stopCompOp, cursor);	//recursively visit child.
		if(e < 0) ERR(e);
	}
	else if((apage->any.hdr.type & LEAF) == LEAF){	//its a leaf.
		found = edubtm_BinarySearchLeaf(apage, kdesc, startKval, &idx);		//found == TRUE : equal, FALSE : less.
		if(idx == -1){	//all the keys in the page are LARGER than startKval.
			if(startCompOp == SM_GT || startCompOp == SM_GE){
				//idx = 0;
			}
			else{
				cursor->flag = CURSOR_EOS;
			}
			//cursor->flag = CURSOR_EOS;
		}
		switch(startCompOp){
			case SM_EQ :
				if(found == FALSE){
					cursor->flag = CURSOR_EOS;
				}
				break;
			case SM_LT :
				if(found){
					idx--;
				}
				if(idx < 0){
					printf("slotNo < 0..\n");
					if(apage->bl.hdr.prevPage == -1){
						cursor->flag = CURSOR_EOS;
					}
					else{
						MAKE_PAGEID(prevPid, root->volNo, apage->bl.hdr.prevPage);
						e = edubtm_Fetch(&prevPid, kdesc, startKval, startCompOp, stopKval, stopCompOp, cursor);
						if(e < 0) ERR(e);
						e = BfM_FreeTrain((TrainID*) root, PAGE_BUF);
						if(e < 0) ERR(e);
						return(eNOERROR);
					}
				}
				break;
			case SM_LE :
				break;
			case SM_GT :
				idx++;
				if(idx >= apage->bl.hdr.nSlots){
					printf("slotNo >= nSlots..\n");
					if(apage->bl.hdr.nextPage == -1){
						printf("Next page is NIL.\n");
						cursor->flag = CURSOR_EOS;
					}
					else{
						printf("Next page is %d.\n",apage->bl.hdr.nextPage);
						MAKE_PAGEID(nextPid, root->volNo, apage->bl.hdr.nextPage);
						e = edubtm_Fetch(&nextPid, kdesc, startKval, startCompOp, stopKval, stopCompOp, cursor);
						if(e < 0) ERR(e);
						e = BfM_FreeTrain((TrainID*) root, PAGE_BUF);
						if(e < 0) ERR(e);
						return(eNOERROR);
					}
				}
				break;
			case SM_GE :
				if(!found){
					idx++;
				}
				if(idx >= apage->bl.hdr.nSlots){
					if(apage->bl.hdr.nextPage == -1){
						cursor->flag = CURSOR_EOS;
					}
					else{
						MAKE_PAGEID(nextPid, root->volNo, apage->bl.hdr.nextPage);
						e = edubtm_Fetch(&nextPid, kdesc, startKval, startCompOp, stopKval, stopCompOp, cursor);
						if(e < 0) ERR(e);
						e = BfM_FreeTrain((TrainID*) root, PAGE_BUF);
						if(e < 0) ERR(e);
						return(eNOERROR);
					}
				}
				break;
		}
		if(cursor->flag == CURSOR_EOS){
			e = BfM_FreeTrain((TrainID*) root, PAGE_BUF);
			if(e < 0) ERR(e);
			return(eNOERROR);
		}
		lEntryOffset = apage->bl.slot[-idx];
		lEntry = &apage->bl.data[lEntryOffset];
		cmp = edubtm_KeyCompare(kdesc, (KeyValue*) &lEntry->klen, stopKval);
		switch(stopCompOp){
			case SM_EQ:
				if(cmp != EQUAL){
					cursor->flag = CURSOR_EOS;
				}
				break;
			case SM_LT:
				if(cmp != LESS){
					cursor->flag = CURSOR_EOS;
				}
				break;
			case SM_LE:
				if(cmp != LESS && cmp != EQUAL){
					cursor->flag = CURSOR_EOS;
				}
				break;
			case SM_GT:
				if(cmp != GREATER){
					cursor->flag = CURSOR_EOS;
				}
				break;
			case SM_GE:
				if(cmp != GREATER && cmp != EQUAL){
					cursor->flag = CURSOR_EOS;
				}
				break;
		}
		if(cursor->flag != CURSOR_EOS){	//stop condition satisfied. return this object.
			cursor->flag = CURSOR_ON;
			cursor->leaf = *root;
			cursor->slotNo = idx;
			memcpy(&cursor->key, &lEntry->klen, sizeof(Two) + lEntry->klen);
			alignedKlen = ALIGNED_LENGTH(lEntry->klen);
			memcpy(&cursor->oid, &lEntry->kval[alignedKlen], sizeof(ObjectID));
		}
		//4. free the buffer.
		e = BfM_FreeTrain((TrainID*) root, PAGE_BUF);
		if(e < 0) ERR(e);
	}
	/* ENDOFNEWCODE */


    return(eNOERROR);
    
} /* edubtm_Fetch() */

