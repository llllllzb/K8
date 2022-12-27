#ifndef APP_INSTRUCTION_H
#define APP_INSTRUCTION_H

#include <stdint.h>
#include "app_sys.h"
#include "app_atcmd.h"

void doParamInstruction(ITEM * item,instructionMode_e mode,char * telnum);
void doStatusInstruction(ITEM * item,instructionMode_e mode,char * telnum);
void doSNInstruction(ITEM * item,instructionMode_e mode,char * telnum);
void doServerInstruction(ITEM * item,instructionMode_e mode,char * telnum);
void doVersionInstruction(ITEM * item,instructionMode_e mode,char * telnum);
void doHbtInstruction(ITEM * item,instructionMode_e mode,char * telnum);
void doModeInstruction(ITEM * item,instructionMode_e mode,char * telnum);
void doTTSInstruction(ITEM * item,instructionMode_e mode,char * telnum);
void doJtInstruction(ITEM * item,instructionMode_e mode,char * telnum);
void dorequestSend123(void);
void do123Instruction(ITEM * item,instructionMode_e mode,char * telnum);
void doAPNInstruction(ITEM * item,instructionMode_e mode,char * telnum);
void doUPSInstruction(ITEM * item,instructionMode_e mode,char * telnum);
void doLOWWInstruction(ITEM * item,instructionMode_e mode,char * telnum);
void doLEDInstruction(ITEM * item,instructionMode_e mode,char * telnum);
void doPOITYPEInstruction(ITEM * item,instructionMode_e mode,char * telnum);
void doResetInstruction(ITEM * item,instructionMode_e mode,char * telnum);
void doUTCInstruction(ITEM * item,instructionMode_e mode,char * telnum);
void doAlarmModeInstrucion(ITEM * item,instructionMode_e mode,char * telnum);
void doDebugInstrucion(ITEM * item,instructionMode_e mode,char * telnum);
void doACCCTLGNSSInstrucion(ITEM * item,instructionMode_e mode,char * telnum);
void doPdopInstrucion(ITEM * item,instructionMode_e mode,char * telnum);
void doSetblemacInstrucion(ITEM * item,instructionMode_e mode,char * telnum);
void doBFInstruction(ITEM * item, instructionMode_e mode, char * telnum);
void doCFInstruction(ITEM * item, instructionMode_e mode, char * telnum);
void doFactoryTestInstruction(ITEM * item, instructionMode_e mode, char * telnum);
void doFenceInstrucion(ITEM * item, instructionMode_e mode, char * telnum);
void doFactoryInstrucion(ITEM * item, instructionMode_e mode, char * telnum);
void doSmsreplyInstrucion(ITEM * item, instructionMode_e mode, char * telnum);
void doJTCYCLEInstrucion(ITEM *item, instructionMode_e mode, char *telnum);
void doAnswerInstrucion(ITEM *item, instructionMode_e mode, char *telnum);
void doTurnAlgInstrucion(ITEM *item, instructionMode_e mode, char *telnum);
void doAdccalInstrucion(ITEM *item, instructionMode_e mode, char *telnum);
void doSetAgpsInstruction(ITEM *item, instructionMode_e mode, char *telnum);
void doAudioInstrucion(ITEM *item, instructionMode_e mode, char *telnum);
void doVolInstrucion(ITEM *item, instructionMode_e mode, char *telnum);
void doVibrangeInstrucion(ITEM *item, instructionMode_e mode, char *telnum);
void doStaticTimeInstrucion(ITEM *item, instructionMode_e mode, char *telnum);

#endif
