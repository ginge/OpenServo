/*
   Copyright (c) 2006, Stefan Engelke <contact@stefanengelke.de>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.

   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

#include <windows.h>
#include "../i2c/StdAfx.h"
#include <jni.h>
#include <string.h>
#include "i2c_U2CWrapper.h"

/*
 * Reads the device handler saved in the given U2CDevice-Object and konverts
 * it into a pointer.
 */
void* getObjHandler
  (JNIEnv *env, jobject devObj) {
    jclass    cls  = env->GetObjectClass(devObj);
    jmethodID id   = env->GetMethodID(cls,"getHandler","()J");
    jlong     jHdl = env->CallLongMethod(devObj,id);
    return (void*)jHdl;
}
/*
 * Konverts a pointer to a java long and writes it into the U2CDevice-Object, to
 * link it with the U2C-device. 
 */
void setObjHandler
  (JNIEnv *env, jobject devObj, void* hdl) {
    jlong jHdl=(jlong)hdl;
    jclass    cls  = env->GetObjectClass(devObj);
    jmethodID id   = env->GetMethodID(cls,"setHandler","(J)V");
    env->CallVoidMethod(devObj,id,jHdl);
}

/*
 * Class:     i2c_U2CWrapper
 * Method:    getDeviceCount
 * Signature: ()B
 */
JNIEXPORT jbyte JNICALL Java_i2c_U2CWrapper_getDeviceCountN
  (JNIEnv *env, jclass clazz) {
    return (jint)U2C_GetDeviceCount();
}

/*
 * Class:     i2c_U2CWrapper
 * Method:    openDeviceN
 * Signature: (ILi2c/U2CDevice;)I
 */
JNIEXPORT jint JNICALL Java_i2c_U2CWrapper_openDeviceN
  (JNIEnv *env, jclass clazz, jint number, jobject devObj) {
    void* hdl=U2C_OpenDevice((byte)number);
    if (hdl==INVALID_HANDLE_VALUE) return U2C_HARDWARE_NOT_FOUND;
    setObjHandler(env,devObj,hdl);
    return U2C_SUCCESS;
}

/*
 * Class:     i2c_U2CWrapper
 * Method:    openDeviceBySerialNumN
 * Signature: (JLi2c/U2CDevice;)I
 */
JNIEXPORT jint JNICALL Java_i2c_U2CWrapper_openDeviceBySerialNumN
  (JNIEnv *env, jclass clazz, jlong serial, jobject devObj) {
  void* hdl=U2C_OpenDeviceBySerialNum((long)serial);
  if (hdl==INVALID_HANDLE_VALUE) return U2C_HARDWARE_NOT_FOUND;
  setObjHandler(env,devObj,hdl);
  return U2C_SUCCESS;
}

/*
 * Class:     i2c_U2CWrapper
 * Method:    closeDeviceN
 * Signature: (Li2c/U2CDevice;)I
 */
JNIEXPORT jint JNICALL Java_i2c_U2CWrapper_closeDeviceN
  (JNIEnv *env, jclass clazz, jobject devObj) {
    void* hdl=getObjHandler(env,devObj);
    return (jint)U2C_CloseDevice(hdl);
}

/*
 * Class:     i2c_U2CWrapper
 * Method:    getDllVersion
 * Signature: ()Li2c/U2CVersionInfo;
 */
JNIEXPORT void JNICALL Java_i2c_U2CWrapper_getDllVersionN
  (JNIEnv *env, jclass clazz, jobject vers) {
    U2C_VERSION_INFO version = U2C_GetDllVersion();
    jclass      cls = env->GetObjectClass(vers);
    jmethodID   id  = env->GetMethodID(cls,"setVersion","(II)V");
    env->CallVoidMethod(vers,id,version.MajorVersion,version.MinorVersion);
}

/*
 * Class:     i2c_U2CWrapper
 * Method:    getSerialNumN
 * Signature: (Li2c/U2CDevice;)I
 */
JNIEXPORT jint JNICALL Java_i2c_U2CWrapper_getSerialNumN
  (JNIEnv *env, jclass clazz, jobject devObj) {
    void* hdl = getObjHandler(env,devObj);
    long serial;
    int status=U2C_GetSerialNum (hdl,&serial);
    if (status!=U2C_SUCCESS) return status;
    jclass    cls = env->GetObjectClass(devObj);
    jmethodID id  = env->GetMethodID(cls,"setSerial","(J)V");
    env->CallVoidMethod(devObj,id,(jlong)serial);
    return status;
}

/*
 * Class:     i2c_U2CWrapper
 * Method:    isDeviceValidN
 * Signature: (Li2c/U2CDevice;)I
 */
JNIEXPORT jint JNICALL Java_i2c_U2CWrapper_isDeviceValidN
  (JNIEnv *env, jclass clazz, jobject devObj) {
    void* hdl = getObjHandler(env,devObj);
    return U2C_IsHandleValid(hdl);
}

/*
 * Class:     i2c_U2CWrapper
 * Method:    getFirmwareVersionN
 * Signature: (Li2c/U2CDevice;Li2c/U2CVersionInfo;)I
 */
JNIEXPORT jint JNICALL Java_i2c_U2CWrapper_getFirmwareVersionN
  (JNIEnv *env, jclass clazz, jobject devObj, jobject verObj) {
    void* hdl = getObjHandler(env,devObj);
    U2C_VERSION_INFO version;
    int status = U2C_GetFirmwareVersion(hdl,&version);
    if (status!=U2C_SUCCESS) return status;
    jclass      cls = env->GetObjectClass(verObj);
    jmethodID   id  = env->GetMethodID(cls,"setVersion","(II)V");
    env->CallVoidMethod(verObj,id,version.MajorVersion,version.MinorVersion);
    return status;
}

/*
 * Class:     i2c_U2CWrapper
 * Method:    getDriverVersionN
 * Signature: (Li2c/U2CDevice;Li2c/U2CVersionInfo;)I
 */
JNIEXPORT jint JNICALL Java_i2c_U2CWrapper_getDriverVersionN
  (JNIEnv *env, jclass clazz, jobject devObj, jobject verObj) {
    void* hdl = getObjHandler(env,devObj);
    U2C_VERSION_INFO version;
    int status = U2C_GetDriverVersion(hdl,&version);
    if (status!=U2C_SUCCESS) return status;
    jclass      cls = env->GetObjectClass(verObj);
    jmethodID   id  = env->GetMethodID(cls,"setVersion","(II)V");
    env->CallVoidMethod(verObj,id,version.MajorVersion,version.MinorVersion);
    return status;  
}

/*
 * Class:     i2c_U2CWrapper
 * Method:    setI2cFreqN
 * Signature: (Li2c/U2CDevice;I)I
 */
JNIEXPORT jint JNICALL Java_i2c_U2CWrapper_setI2cFreqN
  (JNIEnv *env, jclass clazz, jobject devObj, jint freq) {
    void* hdl = getObjHandler(env,devObj);
    return U2C_SetI2cFreq(hdl,(byte)freq);
}

/*
 * Class:     i2c_U2CWrapper
 * Method:    getI2cFreqN
 * Signature: (Li2c/U2CDevice;Li2c/WrapedInt;)I
 */
JNIEXPORT jint JNICALL Java_i2c_U2CWrapper_getI2cFreqN
  (JNIEnv *env, jclass clazz, jobject devObj, jobject wrapedInt) {
    void* hdl = getObjHandler(env,devObj);
    byte freq;
    int status = U2C_GetI2cFreq(hdl,&freq);
    if (status!=U2C_SUCCESS) return status;
    jclass      cls = env->GetObjectClass(wrapedInt);
    jmethodID   id  = env->GetMethodID(cls,"set","(I)V");
    env->CallVoidMethod(wrapedInt,id,(jint)freq);
    return status;  
}

/*
 * Class:     i2c_U2CWrapper
 * Method:    setClockSynchN
 * Signature: (Li2c/U2CDevice;Z)I
 */
JNIEXPORT jint JNICALL Java_i2c_U2CWrapper_setClockSynchN
  (JNIEnv *env, jclass clazz, jobject devObj, jboolean enable) {
    void* hdl = getObjHandler(env,devObj);
    return U2C_SetClockSynch(hdl,(bool)enable);
}

/*
 * Class:     i2c_U2CWrapper
 * Method:    getClockSynchN
 * Signature: (Li2c/U2CDevice;Li2c/WrapedInt;)I
 */
JNIEXPORT jint JNICALL Java_i2c_U2CWrapper_getClockSynchN
  (JNIEnv *env, jclass clazz, jobject devObj, jobject wrapedInt) {
    void* hdl = getObjHandler(env,devObj);
    boolean enable;
    int status = U2C_GetI2cFreq(hdl,&enable);
    if (status!=U2C_SUCCESS) return status;
    jclass      cls = env->GetObjectClass(wrapedInt);
    jmethodID   id  = env->GetMethodID(cls,"set","(I)V");
    if (enable) env->CallVoidMethod(wrapedInt,id,1);
        else    env->CallVoidMethod(wrapedInt,id,0);
    return status;  
}

/*
 * Class:     i2c_U2CWrapper
 * Method:    readN
 * Signature: (Li2c/U2CDevice;Li2c/U2CTransaction;)I
 */
JNIEXPORT jint JNICALL Java_i2c_U2CWrapper_readN
  (JNIEnv *env, jclass clazz, jobject devObj, jobject trans, jbyteArray jBuffer) {
    void* hdl = getObjHandler(env,devObj);
    jbyte *body = env->GetByteArrayElements(jBuffer, 0);
    jclass cls  = env->GetObjectClass(trans);
    jfieldID id;
    id = env->GetFieldID(cls,"nBufferLength","I");
    int bufferLength = (int)env->GetIntField(trans,id);
    id = env->GetFieldID(cls,"nMemoryAddress","I");
    int memoryAddress = (int)env->GetIntField(trans,id);
    id = env->GetFieldID(cls,"nMemoryAddressLength","I");
    int memoryAddressLength = (int)env->GetIntField(trans,id);
    id = env->GetFieldID(cls,"nSlaveDeviceAddress","I");
    int slaveDeviceAddress = (int)env->GetIntField(trans,id);
    
    PU2C_TRANSACTION cmd=new U2C_TRANSACTION();
    cmd->nSlaveDeviceAddress=slaveDeviceAddress;
    cmd->nMemoryAddressLength=memoryAddressLength;
    cmd->nMemoryAddress=memoryAddress;
    cmd->nBufferLength=bufferLength;
    
    int status = U2C_Read(hdl,cmd);
    
    jbyte *array = (env->GetByteArrayElements(jBuffer, 0)); 
    memcpy(array,cmd->Buffer,bufferLength);
    env->ReleaseByteArrayElements(jBuffer, array, 0);
    
    delete cmd;
    return status;
}

/*
 * Class:     i2c_U2CWrapper
 * Method:    writeN
 * Signature: (Li2c/U2CDevice;Li2c/U2CTransaction;[B)I
 */
JNIEXPORT jint JNICALL Java_i2c_U2CWrapper_writeN
  (JNIEnv *env, jclass clazz, jobject devObj, jobject trans, jbyteArray jBuffer) {
    void* hdl = getObjHandler(env,devObj);
    jclass cls  = env->GetObjectClass(trans);
    jfieldID id;
    id = env->GetFieldID(cls,"nBufferLength","I");
    int bufferLength = (int)env->GetIntField(trans,id);
    id = env->GetFieldID(cls,"nMemoryAddress","I");
    int memoryAddress = (int)env->GetIntField(trans,id);
    id = env->GetFieldID(cls,"nMemoryAddressLength","I");
    int memoryAddressLength = (int)env->GetIntField(trans,id);
    id = env->GetFieldID(cls,"nSlaveDeviceAddress","I");
    int slaveDeviceAddress = (int)env->GetIntField(trans,id);
    
    PU2C_TRANSACTION cmd=new U2C_TRANSACTION();
    cmd->nSlaveDeviceAddress=slaveDeviceAddress;
    cmd->nMemoryAddressLength=memoryAddressLength;
    cmd->nMemoryAddress=memoryAddress;
    cmd->nBufferLength=bufferLength;
    jbyte *array = (env->GetByteArrayElements(jBuffer, 0)); 
    memcpy(cmd->Buffer,array,bufferLength);
    env->ReleaseByteArrayElements(jBuffer, array, 0);
    
    int status = U2C_Write(hdl,cmd);
    
    delete cmd;
    return status;
}

/*
 * Class:     i2c_U2CWrapper
 * Method:    scanDevicesN
 * Signature: (Li2c/U2CDevice;Li2c/U2CSlaveAdrList;)I
 */
JNIEXPORT jint JNICALL Java_i2c_U2CWrapper_scanForSlavesN
  (JNIEnv *env, jclass clazz, jobject devObj, jobject adrListObj, jbyteArray arrayObj) {
    void* hdl = getObjHandler(env,devObj);
    PU2C_SLAVE_ADDR_LIST adrList=new U2C_SLAVE_ADDR_LIST();
    
    int status = U2C_ScanDevices(hdl,adrList);
    /*int status=U2C_SUCCESS;
    adrList->List[0]=10;
    adrList->List[1]=11;
    adrList->List[2]=12;
    adrList->nDeviceNumber=200;*/
    
    if (status!=U2C_SUCCESS) return status;
    jclass      cls = env->GetObjectClass(adrListObj);
    jfieldID    id  = env->GetFieldID(cls,"nDeviceNumber","I");
    (*env).SetIntField(adrListObj,id,(jint)adrList->nDeviceNumber);
    
    jbyte *array = (env->GetByteArrayElements(arrayObj, 0));    
    memcpy(array,adrList->List,adrList->nDeviceNumber);
    env->ReleaseByteArrayElements(arrayObj, array, 0);
    delete adrList;
    return status;
}