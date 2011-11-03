/*
  This is a module which implements the notion of a dataset. Its is
  designed for the use with scripting languages.
  
  copyright: GPL

  Mark Koennecke, October 2002
*/
#include <stdlib.h>
#include <string.h>
#include "nxdataset.h"

/*-----------------------------------------------------------------------*/
static int getTypeSize(int typecode){
  switch(typecode){
  case NX_FLOAT32:
  case NX_INT32:
  case NX_UINT32:
    return 4;
    break;
  case NX_FLOAT64:
  case NX_INT64:
  case NX_UINT64:
    return 8;
    break;
  case NX_INT16:
  case NX_UINT16:
    return 2;
    break;
  default:
    return 1;
    break;
  }
}
/*-----------------------------------------------------------------------*/
pNXDS createNXDataset(int rank, int typecode, int dim[]){
  pNXDS pNew = NULL;
  int i, length;

  pNew = (pNXDS)malloc(sizeof(NXDS));
  if(pNew == NULL){
    return NULL;
  }

  pNew->dim = (int *)malloc(rank*sizeof(int));
  for(i = 0, length = 1; i < rank; i++){
    length *= dim[i];
  }
  /* add +1 in case of string NULL termination */
  pNew->u.ptr = malloc(length*getTypeSize(typecode)+1);

  if(pNew->dim == NULL || pNew->u.ptr == NULL){
    free(pNew);
    return NULL;
  }
  pNew->rank = rank;
  pNew->type = typecode;
  pNew->format = NULL;
  for(i = 0; i < rank; i++){
    pNew->dim[i] = dim[i];
  }
  pNew->magic = MAGIC;
  /* add +1 in case of string NULL termination  - see above */
  memset(pNew->u.ptr,0,length*getTypeSize(typecode)+1);
  return pNew;
}
/*---------------------------------------------------------------------*/
pNXDS createTextNXDataset(char *name){
  pNXDS pNew = NULL;

  pNew = (pNXDS)malloc(sizeof(NXDS));
  if(pNew == NULL){
    return NULL;
  }
  pNew->dim = (int *)malloc(sizeof(int));
  pNew->u.cPtr = strdup(name);
  if(pNew->dim == NULL || pNew->u.ptr == NULL){
    free(pNew);
    return NULL;
  }
  pNew->rank = 1;
  pNew->type = NX_CHAR;
  pNew->magic = MAGIC;
  pNew->dim[0] = strlen(name);
  return pNew;
}
/*-----------------------------------------------------------------------*/
void  dropNXDataset(pNXDS dataset){
  if(dataset == NULL){
    return;
  }
  if(dataset->magic != MAGIC){
    return;
  }
  if(dataset->dim != NULL){
    free(dataset->dim);
  }
  if(dataset->u.ptr != NULL){
    free(dataset->u.ptr);
  }
  if(dataset->format != NULL){
    free(dataset->format);
  }
  free(dataset);
}
/*-----------------------------------------------------------------------*/
int   getNXDatasetRank(pNXDS dataset){
  if(dataset == NULL){
    return 0;
  }
  if(dataset->magic != MAGIC){
    return 0;
  }
  return dataset->rank;
}
/*-----------------------------------------------------------------------*/
int   getNXDatasetDim(pNXDS dataset, int which){
  if(dataset == NULL){
    return 0;
  }
  if(dataset->magic != MAGIC){
    return 0;
  }
  if(which < 0 || which >= dataset->rank){
    return 0;
  }
  return dataset->dim[which];
}
/*------------------------------------------------------------------------*/
int   getNXDatasetType(pNXDS dataset){
  if(dataset == NULL){
    return 0;
  }
  if(dataset->magic != MAGIC){
    return 0;
  }
  return dataset->type;
}
/*--------------------------------------------------------------------*/
int getNXDatasetLength(pNXDS dataset){
  int length, i;

  if(dataset == NULL){
    return 0;
  }
  if(dataset->magic != MAGIC){
    return 0;
  }
  length = dataset->dim[0];
  for(i = 1; i < dataset->rank; i++){
    length *= dataset->dim[i];
  }
  return length;
}
/*---------------------------------------------------------------------*/
int getNXDatasetByteLength(pNXDS dataset){
  return getNXDatasetLength(dataset)*getTypeSize(dataset->type);
}
/*----------------------------------------------------------------------
  This calculates an arbitray address in C storage order
  -----------------------------------------------------------------------*/
static int calculateAddress(pNXDS dataset, int pos[]){
  int result, mult;
  int i, j;

  result = pos[dataset->rank - 1];
  for(i = 0; i < dataset->rank -1; i++){
    mult = 1;
    for(j = dataset->rank -1; j > i; j--){
      mult *= dataset->dim[j];
    }
    if(pos[i] < dataset->dim[i] && pos[i] > 0){
      result += mult*pos[i];
    }
  }
  return result;
}
/*-----------------------------------------------------------------------*/
double getNXDatasetValue(pNXDS dataset, int pos[]){
  int address;
 
  if(dataset == NULL){
    return 0;
  }
  if(dataset->magic != MAGIC){
    return 0;
  }

  address = calculateAddress(dataset,pos);
  return getNXDatasetValueAt(dataset, address);
}
/*----------------------------------------------------------------------*/
double getNXDatasetValueAt(pNXDS dataset, int address){
  double value;

  if(dataset == NULL){
    return 0;
  }
  if(dataset->magic != MAGIC){
    return 0;
  }

  switch(dataset->type){
  case NX_FLOAT64:
    value = dataset->u.dPtr[address];
    break;
  case NX_FLOAT32:
    value = (double)dataset->u.fPtr[address];
    break;
  case NX_INT32:
  case NX_UINT32:
    value = (double)dataset->u.iPtr[address];
    break;
  case NX_INT64:
  case NX_UINT64:
    value = (double)dataset->u.lPtr[address];
    break;
  case NX_INT16:
  case NX_UINT16:
    value = (double)dataset->u.sPtr[address];
    break;
  default:
    value = (double)dataset->u.cPtr[address];
    break;
  }
  return value;
}
/*-----------------------------------------------------------------------*/
char  *getNXDatasetText(pNXDS dataset){
  char *resultBuffer = NULL;
  int status = 1;

  if(dataset == NULL){
    return strdup("NULL");
  }
  if(dataset->magic != MAGIC){
    return strdup("NULL");
  }
  if(dataset->rank > 1){
    status = 0;
  } 
  if(dataset->type == NX_FLOAT32 ||
     dataset->type == NX_FLOAT64 ||
     dataset->type == NX_INT32 ||
     dataset->type == NX_UINT32 ||
     dataset->type == NX_INT64 ||
     dataset->type == NX_UINT64 ||
     dataset->type == NX_INT16 ||
     dataset->type == NX_UINT16 ) {
    status = 0;
  }

  if(status == 0){
    return strdup("NO type problem");
  }else{
    resultBuffer = (char *)malloc((dataset->dim[0]+10)*sizeof(char));
    if(resultBuffer == NULL){
      return strdup("NO Memory");
    }
    memset(resultBuffer,0,(dataset->dim[0]+10)*sizeof(char));
    strncpy(resultBuffer,dataset->u.cPtr,dataset->dim[0]);
  }
  return resultBuffer;
}
/*----------------------------------------------------------------------*/
int   putNXDatasetValue(pNXDS dataset, int pos[], double value){
  int address;

  if(dataset == NULL){
    return 0;
  }
  if(dataset->magic != MAGIC){
    return 0;
  }

  address = calculateAddress(dataset,pos);
  return putNXDatasetValueAt(dataset,address,value);
}
  /*---------------------------------------------------------------------*/
int putNXDatasetValueAt(pNXDS dataset, int address, double value){
  /*
    this code is dangerous, it casts without checking the data range.
    This may cause trouble in some cases
  */
  switch(dataset->type){
  case NX_FLOAT64:
    dataset->u.dPtr[address] = value;
    break;
  case NX_FLOAT32:
    dataset->u.fPtr[address] = (float)value;
    break;
  case NX_INT32:
  case NX_UINT32:
    dataset->u.iPtr[address] = (int)value;
    break;
  case NX_INT64:
  case NX_UINT64:
    dataset->u.lPtr[address] = (int64_t)value;
    break;
  case NX_INT16:
  case NX_UINT16:
    dataset->u.sPtr[address] = (short int)value;
    break;
  default:
    dataset->u.cPtr[address] = (char)value;
    break;
  }
  return 1;
}



