/*
 * Errors.h
 *
 *  Created on: Sep 17, 2010
 *      Author: daniel
 */

#ifndef ERRORS
#define ERRORS

/* status and error codes */
#define OK	0
#define ERR_SUP_INVDEV (-101) //invalid device id
#define ERR_SUP_INVOPC (-102) //invalid op code
#define ERR_SUP_INVPOS (-103) //invalid character position
#define ERR_SUP_RDFAIL (-104) //read failed
#define ERR_SUP_WRFAIL (-105) //write failed
#define ERR_SUP_INVMEM (-106) //invalid memory block pointer
#define ERR_SUP_FRFAIL (-107) //free failed
#define ERR_SUP_INVDAT (-108) //invalid date
#define ERR_SUP_DATNCH (-109) //date not changed
#define ERR_SUP_INVDIR (-110) //invalid directory name
#define ERR_SUP_DIROPN (-111) //directory open error
#define ERR_SUP_DIRNOP (-112) //no directory is open
#define ERR_SUP_NOENTR (-113) //no more directory entrie
#define ERR_SUP_NAMLNG (-114) //name too long for buffer
#define ERR_SUP_DIRCLS (-115) //directory close error
#define ERR_SUP_LDFAIL (-116) //program load failed
#define ERR_SUP_FILNFD (-117) //file not found
#define ERR_SUP_FILINV (-118) //file invalid
#define ERR_SUP_PROGSZ (-119) //program size error
#define ERR_SUP_LDADDR (-120) //invalid load address
#define ERR_SUP_NOMEM  (-121) //memory allocation error
#define ERR_SUP_MFREE  (-122) //memory free error
#define ERR_SUP_INVHAN (-123) //invalid handler address
#define ERR_INV_MONTH  (-124) //invalid month
#define ERR_INV_YEAR   (-125) //invalid year
#define ERR_INV_DAY    (-126) //invalid day

//Queue Error Codes

//PCB Error Codes
#define ERR_PCB_NMETOLONG 	(-400) //Name too long
#define ERR_PCB_NONAME   	(-401) //No name supplied
#define ERR_PCB_NMEEXISTS	(-402) //Name already exists
#define ERR_PCB_INVPRIORITY	(-403) //Invalid Priority
#define ERR_PCB_INVCLASS	(-404) //Invalid Class

#endif /* ERRORS_H_ */
