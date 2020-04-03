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
 * Module: edubfm_AllocTrain.c
 *
 * Description : 
 *  Allocate a new buffer from the buffer pool.
 *
 * Exports:
 *  Four edubfm_AllocTrain(Four)
 */


#include <errno.h>
#include "EduBfM_common.h"
#include "EduBfM_Internal.h"


extern CfgParams_T sm_cfgParams;

/*@================================
 * edubfm_AllocTrain()
 *================================*/
/*
 * Function: Four edubfm_AllocTrain(Four)
 *
 * Description : 
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Allocate a new buffer from the buffer pool.
 *  The used buffer pool is specified by the parameter 'type'.
 *  This routine uses the second chance buffer replacement algorithm
 *  to select a victim.  That is, if the reference bit of current checking
 *  entry (indicated by BI_NEXTVICTIM(type), macro for
 *  bufInfo[type].nextVictim) is set, then simply clear
 *  the bit for the second chance and proceed to the next entry, otherwise
 *  the current buffer indicated by BI_NEXTVICTIM(type) is selected to be
 *  returned.
 *  Before return the buffer, if the dirty bit of the victim is set, it 
 *  must be force out to the disk.
 *
 * Returns;
 *  1) An index of a new buffer from the buffer pool
 *  2) Error codes: Negative value means error code.
 *     eNOUNFIXEDBUF_BFM - There is no unfixed buffer.
 *     some errors caused by fuction calls
 */
Four edubfm_AllocTrain(
    Four 	type)			/* IN type of buffer (PAGE or TRAIN) */
{
	/* These local variables are used in the solution code. However, you don¡¯t have to use all these variables in your code, and you may also declare and use additional local variables if needed. */
    Four 	e;			/* for error */
    Four 	victim;			/* return value */
    Four 	i;
    

	/* Error check whether using not supported functionality by EduBfM */
	if(sm_cfgParams.useBulkFlush) ERR(eNOTSUPPORTED_EDUBFM);
	
	/* NEWCODE */
	//1. Buffer-Replacement Algorithm.
	Two n;
	n = BI_NBUFS(type);
	victim = BI_NEXTVICTIM(type);
	
	printf("victim : %d, n : %d\n", victim, n);
	
	for(i = 0; i < 2*n; i++){	//take 2 passes.
		printf("loop #. %d	victim = %d\n", i, victim);
		if(BI_FIXED(type, victim) != 0){	//skip if element is FIXED.
			printf("current entry is fixed!\n");
			victim = (victim + 1) % n;
			continue;
		}
		if((BI_BITS(type, victim) & REFER) == 0){	//if REFER == 0.
			//printf("current entry (bits = %X) is not REFERed!! allocating...\n", BI_BITS(type, victim));
			if((BI_BITS(type, victim) & DIRTY) == DIRTY){
				//printf("needs flushing...\n");
				edubfm_FlushTrain(&(BI_KEY(type, victim)), type);	//flush the original train.
				//printf("flush successful!\n");
			}
			bufInfo[type].bufTable[victim].bits = ALL_0;	//reset bits.
			//printf("bits reset to %X\n", BI_BITS(type, victim));
			bufInfo[type].nextVictim = ((victim + 1) % n);	//set new nextvictim.
			//printf("set new nextVictim value to %d.\n", ((victim + 1) % n));
			//edubfm_Delete(&(BI_KEY(type, victim)), type);	//delete from hash table.
			bfm_Delete(&(BI_KEY(type, victim)), type);
			//printf("deleted key from hash table.\n");
			break;
		}
		else{	//if REFER != 0 -> set REFER to 0 and continue.
			printf("current entry is REFERed, bit = %X. resetting & continue...\n", BI_BITS(type, victim));
			bufInfo[type].bufTable[victim].bits = bufInfo[type].bufTable[victim].bits & ~(REFER);
			printf("bit is changed to %X\n", BI_BITS(type, victim));
		}
		victim = (victim + 1) % n;
	}
	printf("exited loop successfully. i = %d\n",i);
	if(i == (2*n)) ERR(eNOUNFIXEDBUF_BFM);
	/* ENDOFNEWCODE */


    
    return( victim );
    
}  /* edubfm_AllocTrain */
