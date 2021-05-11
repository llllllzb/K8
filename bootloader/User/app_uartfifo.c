#include "app_uartfifo.h"

NODEDATA * headNode=NULL;

uint8_t CreateNodeCmd(char *data,uint16_t datalen,uint8_t currentcmd)
{
    NODEDATA *nextnode;
    NODEDATA *currentnode;
    //uint16_t node=0;
    //char debug[100];
    //如果链表头未创建，则创建链表头。
    if(headNode==NULL)
    {
        headNode=malloc(sizeof(NODEDATA));

        if(headNode!=NULL)
        {
            headNode->currentcmd=currentcmd;
            headNode->data=NULL;
            headNode->data=malloc(datalen);
            if(headNode->data!=NULL)
            {
                memcpy(headNode->data,data,datalen);
                headNode->datalen=datalen;
                headNode->nextnode=NULL;
                //sprintf(debug,"%s==>Create head node at 0x%X successfully\n",__FUNCTION__,headNode);
                //LogMessage(debug);
                return 1;
            }
            else
            {
                free(headNode);
                headNode=NULL;
                //sprintf(debug,"%s==>Can't malloc memorry for head node data\n",__FUNCTION__);
                //LogMessage(debug);
                return 0;
            }
        }
        else
        {
            //sprintf(debug,"%s==>Can't malloc memorry for head node\n",__FUNCTION__);
            //LogMessage(debug);
            return 0;
        }
    }
    currentnode=headNode;
    do
    {
        nextnode=currentnode->nextnode;
        //sprintf(debug,"CreateNodeCmd==>find %d node\n",++node);
        //LogMessage(debug);
        if(nextnode==NULL)
        {
            nextnode=malloc(sizeof(NODEDATA));
            if(nextnode!=NULL)
            {

                nextnode->currentcmd=currentcmd;
                nextnode->data=NULL;
                nextnode->data=malloc(datalen);
                if(nextnode->data!=NULL)
                {
                    memcpy(nextnode->data,data,datalen);
                    nextnode->datalen=datalen;
                    nextnode->nextnode=NULL;
                    // HAL_UART_Transmit(&huart1,(uint8_t *)data,datalen,100);
                    currentnode->nextnode=nextnode;
                    //sprintf(debug,"%s==>Create node at 0x%X successfully\n",__FUNCTION__,nextnode);
                    //LogMessage(debug);
                    nextnode=nextnode->nextnode;
                }
                else
                {
                    free(nextnode);
                    nextnode=NULL;
                    //sprintf(debug,"%s==>Can't malloc memorry for node data\n",__FUNCTION__);
                    //LogMessage(debug);
                    return 0;
                }
            }
            else
            {
                //sprintf(debug,"%s==>Can't malloc memory for node\n",__FUNCTION__);
                //LogMessage(debug);
                return 0;
            }
        }

        currentnode=nextnode;
    }
    while(nextnode!=NULL);

    return 1;
}


void outPutNodeCmd(void)
{
    NODEDATA *nextnode;
    NODEDATA *currentnode;
    if(headNode==NULL)
        return ;
    currentnode=headNode;
    if(currentnode!=NULL)
    {
        nextnode=currentnode->nextnode;
        n58_lte_status.current_cmd=currentnode->currentcmd;
        //uartBufPush(&usart2_ctl,(uint8_t *)currentnode->data, currentnode->datalen);
        HAL_UART_Transmit(&huart2, (uint8_t *)currentnode->data, currentnode->datalen, 1000);
        if(currentnode->data[0]!=0X78 && currentnode->data[0]!=0x79)
        {
            if(n58_lte_status.current_cmd==N58_TTSPLAY_CMD)
            {
                LogMessage("TTS:");
                LogMessageWL(currentnode->data, currentnode->datalen);
                LogMessage("\n");
            }
            else if(n58_lte_status.current_cmd!=N58_FSWF_CMD)
            {
                LogMessageWL(currentnode->data, currentnode->datalen);
            }


        }
        free(currentnode->data);
        free(currentnode);
    }
    headNode=nextnode;
}

