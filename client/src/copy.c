#include "copy.h"
#include "error.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

const char *synFlag = "SYN";
const char *ackFlag = "ACK";
struct Packet {
    char flag[9];
    int seq;
    int ack;
    int dataLength;
    int windowNum;
    char data[1981]
};

char* makePacketHeader(char* flag, int seq, int ack, int window, char* dataToSend);
void processPacketHeader1(char *packet1, struct Packet *clientReceiveBuffer1);

void copy(int from_fd, int to_fd, size_t count)
{
    char dataToSend[1981] = {0};
    char outboundPacket[2041 + 1] = {0};
    char receivedPacket[2041 + 1] = {0};
    ssize_t rbytes;
    ssize_t wbytes;
    int sentDataBytes = 0;
    int receivedDataBytes = 0;
    struct Packet clientReceiveBuffer[25] = {0};

    while((rbytes = read(from_fd, dataToSend, count)) > 0)
    {
        rbytes = strlen(dataToSend) + 61 + 1;

        /**
         * Make packet header
         */
         strcpy (outboundPacket, makePacketHeader(synFlag, sentDataBytes, receivedDataBytes, 58789, dataToSend));


        /**
         * Send packet
         */
        wbytes = write(to_fd, outboundPacket, strlen(outboundPacket));

        /**
         * Wait for ack packet
         */
        rbytes = read(to_fd, receivedPacket, count);
        printf("%s\n", receivedPacket);

        /**
         * Process ack packet header
         */
        processPacketHeader1(receivedPacket, clientReceiveBuffer);
        sentDataBytes = clientReceiveBuffer[0].ack;
        if (strlen(clientReceiveBuffer->data) == clientReceiveBuffer->dataLength) {
            receivedDataBytes += strlen(clientReceiveBuffer->data);
        } else {
            /**
             * data corrupted action
             */
            printf("Packet corrupted\n", receivedPacket);
        }

        memset(dataToSend,0,strlen(dataToSend));
        memset(outboundPacket,0,strlen(outboundPacket));
        memset(receivedPacket,0,strlen(receivedPacket));

        if(wbytes == -1)
        {
            fatal_errno(__FILE__, __func__ , __LINE__, errno, 4);
        }
    }

    if(rbytes == -1)
    {
        fatal_errno(__FILE__, __func__ , __LINE__, errno, 3);
    }

    free(dataToSend);
    free(outboundPacket);
    free(receivedPacket);
}

/**
 * Packet header design:
 * In order
 * Flag 8 chars 0-7 , Sequence Number 16 chars 8-23, Ack Number 16 chars 24-39, Data Length 5 chars 40-44, Window 16 chars 45-60
 * Total 61 chars for packet header
 * Example header: 00000SYN00000000000125380000000000000001000020000000000058789
 */
char* makePacketHeader(char* flag, int seq, int ack, int window, char* dataToSend) {

    char packetHeader[61 + 1] = "0000000000000000000000000000000000000000000000000000000000000";
    int dataLength = strlen(dataToSend);
    char resultingPacket[61 + dataLength + 1];
    //Convert sequence number, ack number, dataLength, window to string
    char seqInString[16];
    char ackInString[16];
    char dataLengthInString[5];
    char windowInString[16];

    sprintf(seqInString, "%d", seq);
    sprintf(ackInString, "%d", ack);
    sprintf(dataLengthInString, "%d", dataLength);
    sprintf(windowInString, "%d", window);

    /**
     * Process flag
     */
    int counter = 0;
    for(int j = 8 - strlen(flag); j < 8; j++) {
        packetHeader[j] = flag[counter];
        counter++;
    }

    counter = 0;
    for(int j = 24 - strlen(seqInString); j < 24; j++) {
        packetHeader[j] = seqInString[counter];
        counter++;
    }

    counter = 0;
    for(int j = 40 - strlen(ackInString); j < 40; j++) {
        packetHeader[j] = ackInString[counter];
        counter++;
    }

    counter = 0;
    for(int j = 45 - strlen(dataLengthInString); j < 45; j++) {
        packetHeader[j] = dataLengthInString[counter];
        counter++;
    }

    counter = 0;
    for(int j = 61 - strlen(windowInString); j < 61; j++) {
        packetHeader[j] = windowInString[counter];
        counter++;
    }
    packetHeader[61] = '\0';
    strcpy(resultingPacket, packetHeader);
    strcat(resultingPacket, dataToSend);
    char * result = {0};
    result = resultingPacket;
    result[61 + dataLength] = '\0';
    return result;
}

void processPacketHeader1(char *packet1, struct Packet *clientReceiveBuffer1) {
    struct Packet tempPacket1 = {0};
    char seqTemp[17];
    char ackTemp[17];
    char dataLengthTemp[6];
    char windowNumTemp[17];
    char dataTemp[1981];

    strncpy(tempPacket1.flag, packet1 + 0, 8);
    strncpy(seqTemp, packet1 + 8, 16);
    seqTemp[16] = '\0';
    strncpy(ackTemp, packet1 + 24, 16);
    ackTemp[16] = '\0';
    strncpy(dataLengthTemp, packet1 + 40, 5);
    dataLengthTemp[5] = '\0';
    strncpy(windowNumTemp, packet1 + 45, 16);
    windowNumTemp[16] = '\0';

    tempPacket1.seq = strtoumax(seqTemp, NULL, 10);
    tempPacket1.ack = strtoumax(ackTemp, NULL, 10);
    tempPacket1.dataLength = strtoumax(dataLengthTemp, NULL, 10);
    tempPacket1.windowNum = strtoumax(windowNumTemp, NULL, 10);
    if (tempPacket1.dataLength < 1981) {
        strncpy(tempPacket1.data, packet1 + 61, tempPacket1.dataLength);
    } else {
        //Throw error
    }
    clientReceiveBuffer1[0] = tempPacket1;
}

