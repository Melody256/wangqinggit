/*
 * cvm_api.h
 *
 *  Created on: 2015-3-28
 *      Author: wintice
 */

#ifndef CVM_API_H_
#define CVM_API_H_
#include "cvm.h"
typedef _CVMPT CVMPT;
typedef _PCVMPT PCVMPT;

int cvmalloc(PCVMPT _pCvmPt);
int cvmpreload(PCVMPT _pCvmPt);
void * cvmload(PCVMPT _pCvmPt);
int cvmsave(PCVMPT _pCvmPt);
int cvmunload(PCVMPT _pCvmPt);
int cvmfree(PCVMPT _pCvmPt);
#endif /* CVM_API_H_ */
