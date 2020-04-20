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
 * Module : eduom_CreateObject.c
 * 
 * Description :
 *  eduom_CreateObject() creates a new object near the specified object.
 *
 * Exports:
 *  Four eduom_CreateObject(ObjectID*, ObjectID*, ObjectHdr*, Four, char*, ObjectID*)
 */


#include <string.h>
#include "EduOM_common.h"
#include "RDsM.h"		/* for the raw disk manager call */
#include "BfM.h"		/* for the buffer manager call */
#include "EduOM_Internal.h"



/*@================================
 * eduom_CreateObject()
 *================================*/
/*
 * Function: Four eduom_CreateObject(ObjectID*, ObjectID*, ObjectHdr*, Four, char*, ObjectID*)
 * 
 * Description :
 * (Following description is for original ODYSSEUS/COSMOS OM.
 *  For ODYSSEUS/EduCOSMOS EduOM, refer to the EduOM project manual.)
 *
 *  eduom_CreateObject() creates a new object near the specified object; the near
 *  page is the page holding the near object.
 *  If there is no room in the near page and the near object 'nearObj' is not
 *  NULL, a new page is allocated for object creation (In this case, the newly
 *  allocated page is inserted after the near page in the list of pages
 *  consiting in the file).
 *  If there is no room in the near page and the near object 'nearObj' is NULL,
 *  it trys to create a new object in the page in the available space list. If
 *  fail, then the new object will be put into the newly allocated page(In this
 *  case, the newly allocated page is appended at the tail of the list of pages
 *  cosisting in the file).
 *
 * Returns:
 *  error Code
 *    eBADCATALOGOBJECT_OM
 *    eBADOBJECTID_OM
 *    some errors caused by fuction calls
 */
Four eduom_CreateObject(
    ObjectID	*catObjForFile,	/* IN file in which object is to be placed */
    ObjectID 	*nearObj,	/* IN create the new object near this object */
    ObjectHdr	*objHdr,	/* IN from which tag & properties are set */
    Four	length,		/* IN amount of data */
    char	*data,		/* IN the initial data for the object */
    ObjectID	*oid)		/* OUT the object's ObjectID */
{
	/* These local variables are used in the solution code. However, you don¡¯t have to use all these variables in your code, and you may also declare and use additional local variables if needed. */
    Four        e;		/* error number */
    Four	neededSpace;	/* space needed to put new object [+ header] */
    SlottedPage *apage;		/* pointer to the slotted page buffer */
    Four        alignedLen;	/* aligned length of initial data */
    Boolean     needToAllocPage;/* Is there a need to alloc a new page? */
    PageID      pid;            /* PageID in which new object to be inserted */
    PageID      nearPid;
    Four        firstExt;	/* first Extent No of the file */
    Object      *obj;		/* point to the newly created object */
    Two         i;		/* index variable */
    sm_CatOverlayForData *catEntry; /* pointer to data file catalog information */
    SlottedPage *catPage;	/* pointer to buffer containing the catalog */
    FileID      fid;		/* ID of file where the new object is placed */
    Two         eff;		/* extent fill factor of file */
    Boolean     isTmp;
    PhysicalFileID pFid;
    

    /*@ parameter checking */
    
    if (catObjForFile == NULL) ERR(eBADCATALOGOBJECT_OM);

    if (objHdr == NULL) ERR(eBADOBJECTID_OM);
    
    /* Error check whether using not supported functionality by EduOM */
    if(ALIGNED_LENGTH(length) > LRGOBJ_THRESHOLD) ERR(eNOTSUPPORTED_EDUOM);
	
	/* NEWCODE */
	//0. get Catalog, physicalfile ID, firstExt.
	e = BfM_GetTrain((TrainID*)catObjForFile, (char**)&catPage, PAGE_BUF);
	if(e < 0) ERR(e);
	GET_PTR_TO_CATENTRY_FOR_DATA(catObjForFile, catPage, catEntry);
	fid = catEntry->fid;
	MAKE_PHYSICALFILEID(pFid, catEntry->fid.volNo, catEntry->firstPage);
	e = RDsM_PageIdToExtNo((PageID*)&pFid, &firstExt);
	if(e < 0) ERR(e);
	//1. calculate amount of new space needed.
	alignedLen = 4 * ((length / 4) + 1);
	neededSpace = sizeof(ObjectHdr) + alignedLen + sizeof(SlottedPageSlot);
	//determine which is the right "availspacelist".
	ShortPageID rightlist = catEntry->availSpaceList50;
	if(neededSpace < SP_50SIZE) rightlist = catEntry->availSpaceList40;
	if(neededSpace < SP_40SIZE) rightlist = catEntry->availSpaceList30;
	if(neededSpace < SP_30SIZE) rightlist = catEntry->availSpaceList20;
	if(neededSpace < SP_20SIZE) rightlist = catEntry->availSpaceList10;
	//2. Choose a Page.
	if(nearObj != NULL){
		//get the "near page".
		MAKE_PAGEID(nearPid, nearObj->volNo, nearObj->pageNo);
		e = BfM_GetTrain((TrainID*)&nearPid, (char**)&apage, PAGE_BUF);
		if(e < 0) ERR(e);
		//condition : is there enough room in "nearpage"??
		needToAllocPage = (SP_FREE(apage) < neededSpace);
		if(needToAllocPage){
			//Allocate a new page to "pid", initialize header.
			e = RDsM_AllocTrains(fid.volNo, firstExt, &nearPid, catEntry->eff, 1, PAGESIZE2, &pid);
			if(e < 0) ERR(e);
			e = BfM_FreeTrain(&nearPid, PAGE_BUF);
			if(e < 0) ERR(e);
			e = BfM_GetNewTrain(&pid, (char **)&apage, PAGE_BUF);
			if(e < 0) ERR(e);
			apage->header.pid = pid;
			apage->header.nSlots = 1;
			apage->header.free = 0;
			apage->header.unused = 0;
			apage->header.fid = fid;
			//insert new page as the next page of "nearpage".
			e = om_FileMapAddPage(catObjForFile, &nearPid, &pid);
			if (e < 0) ERRB1(e, &pid, PAGE_BUF);
		}
		else{
			//"nearpage" is the target page, remove this page from AvailList.
			pid = nearPid;
			e = om_RemoveFromAvailSpaceList(catObjForFile, &pid, apage);
			if (e < 0) ERRB1(e, &pid, PAGE_BUF);
			//compact the page.
			EduOM_CompactPage(apage, -1);
		}
	}
	else{
		printf("nearobj is NULL.\n");
		if((neededSpace <= SP_50SIZE) && rightlist != NIL){
			printf("getting page from availspacelist, rightlist == %d\n", rightlist);
			MAKE_PAGEID(pid, fid.volNo, rightlist);
			e = BfM_GetTrain((TrainID*)&pid, (char**)&apage, PAGE_BUF);
			if(e < 0) ERR(e);
			printf("Got page number : %d from rightlist = %d\n", pid.pageNo, rightlist);
			e = om_RemoveFromAvailSpaceList(catObjForFile, &pid, apage);
			if (e < 0) ERRB1(e, &pid, PAGE_BUF);
			EduOM_CompactPage(apage, -1);
			printf("compacted page.\n");
		}
		else{
			printf("getting the last page of the file.\n");
			//get the last page of the file. Check if it has enough free space.
			MAKE_PAGEID(pid, fid.volNo, catEntry->lastPage);
			e = BfM_GetTrain(&pid, (char**)&apage, PAGE_BUF);
			if(e < 0) ERR(e);
			if(SP_FREE(apage) >= neededSpace){
				printf("got last page: %d\n", pid.pageNo);
				//pid = apage->header.pid;
				EduOM_CompactPage(apage, -1);
			}
			else{
				printf("allocating a new page..\n");
				//allocate new page.
				e = BfM_FreeTrain(&pid, PAGE_BUF);
				if(e < 0) ERR(e);
				e = RDsM_AllocTrains(fid.volNo, firstExt, &nearPid, catEntry->eff, 1, PAGESIZE2, &pid);
				if(e < 0) ERR(e);
				e = BfM_GetNewTrain(&pid, (char **)&apage, PAGE_BUF);
				if(e < 0) ERR(e);
				apage->header.pid = pid;
				apage->header.nSlots = 1;
				apage->header.free = 0;
				apage->header.unused = 0;
				apage->header.fid = fid;
				//insert new page as the last page.
				e = om_FileMapAddPage(catObjForFile, &catEntry->lastPage, &pid);
				if (e < 0) ERRB1(e, &pid, PAGE_BUF);
			}
		}
	}
	//3. now "apage" is the target page. insert new object into this page.
	//update header.
	objHdr->length = length;
	//obj->header = *objHdr;
	//copy new object to the continuous free area.
	i = apage->header.free;
	printf("free area start is %d\n", i);
	memcpy(&apage->data[i], objHdr, sizeof(ObjectHdr));
	printf("copied header.\n");
	int j;
	for(j=0; j< length; j++){
		apage->data[i + sizeof(ObjectHdr) + j] = data[j];
		printf("copied char %s to data area.\n", apage->data[i + sizeof(ObjectHdr) + j]);
		//obj->data[j] = data[j];
	}
	//find an empty slot or allocate a new slot.
	for(j=0; j< apage->header.nSlots; j++){
		if(apage->slot[-j].offset == EMPTYSLOT){
			apage->slot[-j].offset = i;
			e = om_GetUnique(&pid, &(apage->slot[-j].unique));
			if (e < 0) ERRB1(e, &pid, PAGE_BUF);
			break;
		}
	}
	printf("new object is saved to slot number %d\n", j);
	if(j == apage->header.nSlots){
		printf("nSlots ++.\n");
		apage->slot[-j].offset = i;
		e = om_GetUnique(&pid, &(apage->slot[-j].unique));
		if (e < 0) ERRB1(e, &pid, PAGE_BUF);
		apage->header.nSlots++;
	}
	//save the new object's id @ "oid"
	MAKE_OBJECTID(*oid, pid.volNo, pid.pageNo, j, apage->slot[-j].unique);
	printf("made OUTPUT object id : v=%d, p=%d, s=%d, u=%d\n", oid->volNo, oid->pageNo, oid->slotNo, oid->unique);
	//4. Update page header & put page back in "availspacelist".
	apage->header.free = apage->header.free + sizeof(ObjectHdr) + alignedLen;
	apage->header.unused = apage->header.unused + alignedLen - length;
	e = om_PutInAvailSpaceList(catObjForFile, &pid, apage);
	if(e < 0) ERRB1(e, &pid, PAGE_BUF);
	//5. Set dirty & free the buffers.
	e = BfM_SetDirty(&pid, PAGE_BUF);
	if(e < 0) ERRB1(e, &pid, PAGE_BUF);
	e = BfM_FreeTrain(&pid, PAGE_BUF);
	if(e < 0) ERR(e);
	e = BfM_FreeTrain((TrainID*)catObjForFile, PAGE_BUF);
	if(e < 0) ERR(e);
	/* ENDOFNEWCODE */



    return(eNOERROR);
    
} /* eduom_CreateObject() */
