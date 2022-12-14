#include "copy.h"
#include "error.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>

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
int getRandNum0To100 ();
void copy(int from_fd, int to_fd, size_t count)
{

    int bytesReceived = 0;
    int receivedPackets = 0;
    int duplicatePackets = 0;
    int bytesSent = 1;
    char receivedPacket[2041+1] = {0};
    char dataToSend[1981] = {0};
    ssize_t rbytes;
    ssize_t wbytes;
    char ackPacket[2041+1] = {0};
    struct Packet clientReceiveBuffer[25] = {0};
    printf("Received messages:\n");
    while((rbytes = read(from_fd, receivedPacket, 2041)) > 0)
    {
        printf("\x1b[1F"); // Move to beginning of previous line
        printf("\x1b[2K"); // Clear entire line
        receivedPackets++;
        /**
         * Parse received packet header to struct
         */
        processPacketHeader1(receivedPacket, clientReceiveBuffer);
        //if (strlen(clientReceiveBuffer[0].data) == clientReceiveBuffer[0].dataLength) { //Check if data is corrupted
            if (clientReceiveBuffer[0].seq == bytesReceived) { //Check if sequence is in order
                bytesReceived += clientReceiveBuffer[0].dataLength;
                 /**
                 * Make use of received packet
                 */

                printf("%s", clientReceiveBuffer[0].data);
            } else {
                duplicatePackets++;
            }
        //}




        /**
         * Make ack packet
         */
        strcpy(dataToSend, "");
        strcpy (ackPacket, makePacketHeader(ackFlag, bytesSent, bytesReceived, 1, dataToSend));

        /**
         * Send Ack to client
         */
         //if (getRandNum0To100() > 70) {
             wbytes = write(from_fd, ackPacket, strlen(ackPacket));
         //} else {
             //printf("Dropped packet ack for %d ack; \n", bytesReceived);
         //}

        printf("Received and acked data: %d bytes; ", bytesReceived);
        printf("Received packets: %d; ", receivedPackets);
        printf("%d / %d duplicate packets received;\n", duplicatePackets, receivedPackets);

        memset(receivedPacket,0,strlen(receivedPacket));
        memset(ackPacket,0,strlen(ackPacket));
        if(wbytes == -1)
        {
            fatal_errno(__FILE__, __func__ , __LINE__, errno, 4);
        }

    }

    if(rbytes == -1)
    {
        fatal_errno(__FILE__, __func__ , __LINE__, errno, 3);
    }
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
    char *result = {0};
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
    if (tempPacket1.dataLength < 1982) {
        strncpy(tempPacket1.data, packet1 + 61, tempPacket1.dataLength);
    } else {
        //Throw error
        fatal_message(__FILE__, __func__ , __LINE__, "Unexpected packet size", 6);
    }
    clientReceiveBuffer1[0] = tempPacket1;
}

int getRandNum0To100 () {
    srand(time(0));
    int i;
    int num = (rand() % (100 - 1 + 1)) + 1;
    return num;
}

