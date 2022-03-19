#ifndef APP_INSTRUCTION_H
#define APP_INSTRUCTION_H

#include <stdint.h>
#include "app_sys.h"
#include "app_atcmd.h"

void doParamInstruction(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doStatusInstruction(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doSNInstruction(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doServerInstruction(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doVersionInstruction(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doHbtInstruction(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doModeInstruction(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doTTSInstruction(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doJtInstruction(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void dorequestSend123(void);
void do123Instruction(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doAPNInstruction(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doUPSInstruction(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doLOWWInstruction(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doLEDInstruction(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doPOITYPEInstruction(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doResetInstruction(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doUTCInstruction(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doAlarmModeInstrucion(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doDebugInstrucion(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doACCCTLGNSSInstrucion(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doPdopInstrucion(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doSetblemacInstrucion(ITEM * item,DOINSTRUCTIONMODE mode,char * telnum);
void doBFInstruction(ITEM * item, DOINSTRUCTIONMODE mode, char * telnum);
void doCFInstruction(ITEM * item, DOINSTRUCTIONMODE mode, char * telnum);
void doFactoryTestInstruction(ITEM * item, DOINSTRUCTIONMODE mode, char * telnum);
void doFenceInstrucion(ITEM * item, DOINSTRUCTIONMODE mode, char * telnum);
void doFactoryInstrucion(ITEM * item, DOINSTRUCTIONMODE mode, char * telnum);
void doSmsreplyInstrucion(ITEM * item, DOINSTRUCTIONMODE mode, char * telnum);
void doJTCYCLEInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum);
void doAnswerInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum);
void doTurnAlgInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum);
void doAdccalInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum);
void doSetAgpsInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum);
void doAudioInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum);
void doVolInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum);


#endif
