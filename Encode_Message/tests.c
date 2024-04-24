#include <stdio.h>
#include "main.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h> 
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define SERVER_IP "fe80::11"
#define SERVER_PORT 15118
#define SHM_SIZE 1024
#define SHM_KEY 1234
void error(const char *msg) {
    perror(msg);
    fprintf(stderr, "Error code: %d\n", errno);
    fprintf(stderr, "Error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
}

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <string.h>

#include "EXITypes.h"

#include "appHandEXIDatatypes.h"
#include "appHandEXIDatatypesEncoder.h"
#include "appHandEXIDatatypesDecoder.h"

/* Activate support for DIN */
#include "dinEXIDatatypes.h"
#if DEPLOY_DIN_CODEC == SUPPORT_YES
#include "dinEXIDatatypesEncoder.h"
#include "dinEXIDatatypesDecoder.h"
#endif /* DEPLOY_DIN_CODEC == SUPPORT_YES */

/* Activate support for XMLDSIG */
#include "xmldsigEXIDatatypes.h"
#if DEPLOY_XMLDSIG_CODEC == SUPPORT_YES
#include "xmldsigEXIDatatypesEncoder.h"
#include "xmldsigEXIDatatypesDecoder.h"
#endif /* DEPLOY_XMLDSIG_CODEC == SUPPORT_YES */

/* Activate support for ISO1 */
#include "iso1EXIDatatypes.h"
#if DEPLOY_ISO1_CODEC == SUPPORT_YES
#include "iso1EXIDatatypesEncoder.h"
#include "iso1EXIDatatypesDecoder.h"
#endif /* DEPLOY_ISO1_CODEC == SUPPORT_YES */

/* Activate support for ISO2 */
#include "iso2EXIDatatypes.h"
#if DEPLOY_ISO2_CODEC == SUPPORT_YES
#include "iso2EXIDatatypesEncoder.h"
#include "iso2EXIDatatypesDecoder.h"
#endif /* DEPLOY_ISO2_CODEC == SUPPORT_YES */
#include <net/if.h>
#include "v2gtp.h"

#define BUFFER_SIZE 800
uint8_t buffer1[BUFFER_SIZE];
uint8_t buffer2[BUFFER_SIZE];

#define ERROR_UNEXPECTED_REQUEST_MESSAGE -601
#define ERROR_UNEXPECTED_SESSION_SETUP_RESP_MESSAGE -602
#define ERROR_UNEXPECTED_SERVICE_DISCOVERY_RESP_MESSAGE -602
#define ERROR_UNEXPECTED_SERVICE_DETAILS_RESP_MESSAGE -603
#define ERROR_UNEXPECTED_PAYMENT_SERVICE_SELECTION_RESP_MESSAGE -604
#define ERROR_UNEXPECTED_PAYMENT_DETAILS_RESP_MESSAGE -605
#define ERROR_UNEXPECTED_AUTHORIZATION_RESP_MESSAGE -606
#define ERROR_UNEXPECTED_CHARGE_PARAMETER_DISCOVERY_RESP_MESSAGE -607
#define ERROR_UNEXPECTED_POWER_DELIVERY_RESP_MESSAGE -608
#define ERROR_UNEXPECTED_CHARGING_STATUS_RESP_MESSAGE -609
#define ERROR_UNEXPECTED_METERING_RECEIPT_RESP_MESSAGE -610
#define ERROR_UNEXPECTED_SESSION_STOP_RESP_MESSAGE -611
#define ERROR_UNEXPECTED_CABLE_CHECK_RESP_MESSAGE -612
#define ERROR_UNEXPECTED_PRE_CHARGE_RESP_MESSAGE -612
#define ERROR_UNEXPECTED_CURRENT_DEMAND_RESP_MESSAGE -613
#define ERROR_UNEXPECTED_WELDING_DETECTION_RESP_MESSAGE -614
int evse_socket;  // Declare the socket variable globally
struct sockaddr_in6 evse_addr;  // Declare the address structure globally
int client_socket;
struct sockaddr_in6 server_addr; 
// Define port for appHandshake function
#define APP_HANDSHAKE_PORT  51111   
#define APP_IP "FE80::11"
#include <netinet/in.h>
uint8_t valueAtIndex19;
uint8_t valueAtIndex22;
uint8_t valueAtIndex;
uint8_t V2GTP_VERSION_INV = 0xFE; 
void handleError(const char *msg) {
    perror(msg);
    fprintf(stderr, "Error code: %d\n", errno);
    fprintf(stderr, "Error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
}

static int writeStringToEXIString(char* string, exi_string_character_t* exiString) {
    int pos = 0;
    while(string[pos]!='\0') {
        exiString[pos] = string[pos];
        pos++;
    }
    return pos;
}
void print_iso2EXIDocument(struct iso2EXIDocument* exiIn) {
    // Print each field of the iso2EXIDocument structure
    printf("V2G_Message_isUsed: %u\n", exiIn->V2G_Message_isUsed);
    // Print other fields similarly...

    // Print the SessionID bytes
    printf("SessionID: ");
    for (int i = 0; i < exiIn->V2G_Message.Header.SessionID.bytesLen; ++i) {
        printf("%02X", exiIn->V2G_Message.Header.SessionID.bytes[i]);
    }
    printf("\n");

}

static void printASCIIString(exi_string_character_t* string, uint16_t len) {
        unsigned int i;
        for(i=0; i<len; i++) {
                printf("%c",(char)string[i]);
        }
        printf("\n");
}

static void printBinaryArray(uint8_t* byte, uint16_t len) {
	unsigned int i;
	for(i=0; i<len; i++) {
		printf("%02X",byte[i]);
	}
	printf("\n");
}
static void copyBytes(uint8_t* from, uint16_t len, uint8_t* to) {
        int i;
        for(i=0; i<len; i++) {
                to[i] = from[i];
        }
}

/* serializes EXI stream and adds V2G TP header */
static int serialize2EXI2Stream(struct iso2EXIDocument* exiIn, bitstream_t* stream, int use1EHeader) {
	int errn;
	*stream->pos = V2GTP_HEADER_LENGTH;  /* v2gtp header */

	if( (errn = encode_iso2ExiDocument(stream, exiIn, use1EHeader)) == 0) {
		errn = write_v2gtpHeader(stream->data, (*stream->pos)-V2GTP_HEADER_LENGTH, V2GTP_EXI_TYPE);
	}else {
        printf("Error occurred during encoding. Error code: %d\n", errn);
    }
	return errn;
}

size_t findByteSequence(const uint8_t* data, size_t dataSize, const uint8_t* sequence, size_t sequenceSize) {
    for (size_t i = 0; i < dataSize - sequenceSize; ++i) {
        if (memcmp(data + i, sequence, sequenceSize) == 0) {
            return i;
        }
    }
    return dataSize;  // Not found
}

/* deserializes V2G TP header and decodes right away EXI stream */
static int deserialize2Stream2EXI(bitstream_t* streamIn, struct iso2EXIDocument* exi, size_t bytesReceived) {
    int errn;
    uint32_t payloadLength;
    // Reset the bitstream to the beginning
    *streamIn->pos = 0; 
    // Ensure that bytesReceived is within the stream size
    if (bytesReceived > streamIn->size) {
        printf("Error: Invalid bytesReceived value.\n");
        return -1; // or another appropriate error code
    }

    // Read V2G TP Header
    errn = read_v2gtpHeader(streamIn->data, &payloadLength);
    if (errn != 0) {
        printf("Error reading V2G TP Header. Error Code: %d\n", errn);
        return errn;
    }

    // Check if bytesReceived is less than the expected payload size
    if (bytesReceived < payloadLength + V2GTP_HEADER_LENGTH) {
        printf("Error: Insufficient data for the specified payload length.\n");
        return -1; // or another appropriate error code
    }
     // Move the stream position past the V2G TP Header
     *streamIn->pos += V2GTP_HEADER_LENGTH;
    // Deserialize only the valid portion of the stream based on payloadLength
    errn = decode_iso2ExiDocument(streamIn, exi);
    return errn;
}

size_t findByte(const uint8_t* array, size_t size, uint8_t target) {
    for (size_t i = 0; i < size; ++i) {
        if (array[i] == target) {
            return i;  // Return the position if found
        }
    }
    return SIZE_MAX;  // Return SIZE_MAX if not found
}
void appendBytes(uint8_t* destination, const uint8_t* source, size_t sourceSize) {
    // Find the null terminator in the destination (assuming it's a string)
    size_t destinationSize = strlen((char*)destination);

    // Copy the source bytes to the end of the destination
    memcpy(destination + destinationSize, source, sourceSize);
}
static int request_response2(struct iso2EXIDocument* exiIn, struct iso2EXIDocument* exiOut, int use1EHeader, size_t size) {
    int errn;

    bitstream_t stream1;
    bitstream_t stream2;

    size_t pos1;
    size_t pos2;

    stream1.size = BUFFER_SIZE;
    stream1.data = buffer1;
    stream1.pos = &pos1;

    stream2.size = BUFFER_SIZE;
    stream2.data = buffer2;
    stream2.pos = &pos2;
    /* EV side */
    pos1 = size;
  errn = serialize2EXI2Stream(exiIn, &stream1,use1EHeader);
size_t position2 = findByte(stream1.data, pos1, 0xD0);
if(pos1 != 44){
stream1.data[19] =  valueAtIndex19;
}
if (position2 != SIZE_MAX) {
    // Replace 0xD0 with 0x98
    stream1.data[position2] = 0x98;
}    // Print the payload before sending
if(pos1 == 23){
  size_t newSize = 38;
    // Allocate memory for the new array
    uint8_t* newArray = (uint8_t*)malloc(newSize);

    if (newArray != NULL) {
        // Copy the contents of the original array to the new array
        memcpy(newArray, stream1.data, 20);
        // Add additional bytes - 
        uint8_t additionalBytes[] = {0xB0, 0x20, 0xEE, 0xEE, 0xEE, 0x5C, 0xEC, 0xCA,
                                      0xC6, 0xE8, 0xDE, 0xE4, 0x5C, 0xC6, 0xDE, 0xDA, 0x00, 0x00};
        memcpy(newArray + 20, additionalBytes, sizeof(additionalBytes));
        // Print the result
        printBinaryArray(newArray, newSize);
    // Send the payload to the EVSE
    if (send(evse_socket, newArray, newSize, 0) == -1) {
        perror("Error sending data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }

    // Print the payload after sending
    printf("Payload sent successfully\n");

    // Receive response from the EVSE
    ssize_t bytes_received = recv(evse_socket, stream2.data, BUFFER_SIZE, 0);
    if (bytes_received == -1) {
        perror("Error receiving data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
    errn = deserialize2Stream2EXI(&stream2, exiOut, bytes_received);
        free(newArray);
    } else {
        printf("Memory allocation failed\n");
    }
}else{
if (pos1 == 25 && stream1.data[22] == 0x24 ) {
    stream1.data[22] = 0x12;
    stream1.data[7] = 0x10;
    pos1 = 24;
}  
  if(pos1 == 53){
   size_t newSize = 1493;
    // Allocate memory for the new array
    uint8_t* newArray = (uint8_t*)malloc(newSize);
    if (newArray != NULL) {
        // Copy the contents of the original array to the new array
        memcpy(newArray, stream1.data, 20);
        // Add additional bytes -
	uint8_t additionalBytes[] = {0x10, 0x45, 0x11, 0x14, 0xE1, 0x05, 0x04, 0xC5, 0x04, 0xC9, 0x08, 0xCD, 0x0C, 0xD1, 0x10, 0xD4, 0xE4, 0x5B, 0x60, 0x66, 0x10, 0x40, 0x3A, 0xE6, 0x10, 0x40, 0x2F, 0xD4, 0x00, 0x60, 0x40, 0x20, 0x40, 0x41, 0x20, 0x11, 0x28, 0xD0, 0x62, 0xB5, 0xE2, 0x63, 0xDA, 0x66, 0x01, 0x40, 0xC1, 0x05, 0x50, 0xC9, 0x19, 0xC7, 0xA0, 0x80, 0x60, 0x46, 0x08, 0x26, 0x22, 0x26, 0x01, 0xE0, 0xC0, 0x6A, 0xA0, 0x80, 0x61, 0x81, 0x09, 0xA9, 0xEA, 0x6E, 0xAC, 0x46, 0x4B, 0xEB, 0x06, 0x23, 0xE6, 0x03, 0xA0, 0xC0, 0x6A, 0xA0, 0x81, 0x41, 0x82, 0xCA, 0xCC, 0xAC, 0x6E, 0x8D, 0xEE, 0x44, 0x09, 0x2D, 0xCC, 0xCD, 0xEE, 0x4D, 0xAC, 0x2E, 0x8D, 0x2D, 0x64, 0x08, 0xED, 0xAC, 0x49, 0x06, 0x21, 0x66, 0x01, 0x20, 0xC0, 0x6A, 0xA0, 0x80, 0xC2, 0x60, 0x48, 0x88, 0xA6, 0x04, 0x02, 0xE1, 0xA6, 0x46, 0x46, 0x06, 0x26, 0x26, 0x46, 0x26, 0x86, 0xA6, 0xE6, 0x26, 0x2B, 0x43, 0x01, 0xE6, 0x46, 0x06, 0xA6, 0x46, 0x06, 0x26, 0x06, 0xA6, 0x26, 0x86, 0xA6, 0xE6, 0x26, 0x2B, 0x46, 0x07, 0xC6, 0x23, 0x66, 0x03, 0x20, 0xC0, 0x6A, 0xA0, 0x80, 0x61, 0x82, 0x48, 0x88, 0xA5, 0xA7, 0x08, 0x28, 0x25, 0xA6, 0x28, 0x26, 0x48, 0x46, 0x68, 0x66, 0x88, 0x86, 0xA5, 0xA7, 0x26, 0x23, 0xE6, 0x03, 0xA0, 0xC0, 0x6A, 0xA0, 0x81, 0x41, 0x82, 0xCA, 0xCC, 0xAC, 0x6E, 0x8D, 0xEE, 0x44, 0x09, 0x2D, 0xCC, 0xCD, 0xEE, 0x4D, 0xAC, 0x2E, 0x8D, 0x2D, 0x64, 0x08, 0xED, 0xAC, 0x49, 0x06, 0x0B, 0x26, 0x02, 0x60, 0xC0, 0xE5, 0x50, 0xC9, 0x19, 0xC7, 0xA0, 0x40, 0x20, 0xC1, 0x05, 0x50, 0xC9, 0x19, 0xC7, 0xA0, 0x60, 0x20, 0xE0, 0x68, 0x40, 0x00, 0x8C, 0x72, 0xC0, 0x26, 0xC4, 0x58, 0x93, 0xC2, 0xE6, 0x4E, 0x76, 0x4C, 0x67, 0xDC, 0xDE, 0x2B, 0xF2, 0x85, 0xD7, 0x31, 0x8A, 0x5C, 0xF6, 0xAF, 0xB1, 0x54, 0x3C, 0xE0, 0x11, 0x2F, 0x04, 0x86, 0x43, 0x41, 0xD0, 0xD7, 0xF0, 0xAB, 0xC3, 0x6D, 0x0D, 0x5C, 0x2A, 0x16, 0xD4, 0x39, 0x32, 0xE2, 0xB6, 0x67, 0x70, 0x23, 0x6F, 0x91, 0x27, 0xB1, 0xB2, 0x50, 0xE8, 0x7A, 0x68, 0xE3, 0x55, 0xCD, 0x94, 0x6C, 0x06, 0x0B, 0xC6, 0x01, 0x80, 0xC0, 0x6A, 0xA3, 0xA2, 0x60, 0x20, 0x3F, 0xE0, 0x80, 0x46, 0x00, 0x06, 0x01, 0xC0, 0xC0, 0x6A, 0xA3, 0xA1, 0xE0, 0x20, 0x3F, 0xE0, 0x80, 0x80, 0x60, 0x40, 0x7D, 0x06, 0x03, 0xE0, 0xC0, 0x6A, 0xA3, 0xA4, 0x60, 0x83, 0x06, 0x02, 0xD0, 0x02, 0x82, 0x71, 0xF2, 0x2D, 0xCF, 0x90, 0x4B, 0xA9, 0x91, 0x95, 0x44, 0xCF, 0x79, 0x84, 0x6D, 0xBE, 0x2D, 0x62, 0x1C, 0xE5, 0xE6, 0x03, 0xA0, 0xC0, 0x6A, 0xA3, 0xA1, 0xC0, 0x82, 0xC0, 0x82, 0x93, 0xFE, 0x8F, 0xFD, 0xB6, 0x8D, 0x66, 0xBE, 0x83, 0x59, 0x37, 0xDF, 0x60, 0x7B, 0x29, 0x18, 0xAA, 0x8F, 0x4E, 0xD8, 0x46, 0x01, 0x40, 0xC1, 0x05, 0x50, 0xC9, 0x19, 0xC7, 0xA0, 0x80, 0x60, 0x40, 0x68, 0xE0, 0x06, 0x08, 0x80, 0x44, 0x0B, 0x92, 0xD1, 0x8A, 0xC1, 0x28, 0xAC, 0xA3, 0x07, 0x52, 0x00, 0xE5, 0xD0, 0x15, 0x7F, 0x69, 0x06, 0x41, 0x83, 0xD9, 0x24, 0xA5, 0x93, 0xA9, 0x1C, 0xAA, 0xCC, 0x3E, 0x8A, 0xB5, 0x0B, 0x79, 0x80, 0x44, 0x0B, 0xCB, 0xAD, 0xD0, 0x9B, 0x0F, 0xFF, 0xB1, 0x15, 0x2C, 0x45, 0x02, 0x3C, 0x88, 0x7A, 0x7F, 0x74, 0x23, 0x53, 0xD6, 0x07, 0x48, 0xAB, 0xD2, 0x90, 0x1B, 0x7D, 0x51, 0xF2, 0x63, 0x7B, 0x42, 0x00, 0xE5, 0x03, 0x30, 0x82, 0x01, 0xE1, 0x30, 0x82, 0x01, 0x87, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x09, 0x00, 0x89, 0x46, 0x83, 0x15, 0xAF, 0x13, 0x1E, 0xD2, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x30, 0x41, 0x31, 0x11, 0x30, 0x0F, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x08, 0x4D, 0x4F, 0x53, 0x75, 0x62, 0x31, 0x5F, 0x58, 0x31, 0x1F, 0x30, 0x1D, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x0C, 0x16, 0x56, 0x65, 0x63, 0x74, 0x6F, 0x72, 0x20, 0x49, 0x6E, 0x66, 0x6F, 0x72, 0x6D, 0x61, 0x74, 0x69, 0x6B, 0x20, 0x47, 0x6D, 0x62, 0x48, 0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x44, 0x45, 0x30, 0x20, 0x17, 0x0D, 0x32, 0x32, 0x30, 0x31, 0x31, 0x32, 0x31, 0x34, 0x35, 0x37, 0x31, 0x30, 0x5A, 0x18, 0x0F, 0x32, 0x30, 0x35, 0x32, 0x30, 0x31, 0x30, 0x35, 0x31, 0x34, 0x35, 0x37, 0x31, 0x30, 0x5A, 0x30, 0x41, 0x31, 0x11, 0x30, 0x0F, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x08, 0x4D, 0x4F, 0x53, 0x75, 0x62, 0x32, 0x5F, 0x58, 0x31, 0x1F, 0x30, 0x1D, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x0C, 0x16, 0x56, 0x65, 0x63, 0x74, 0x6F, 0x72, 0x20, 0x49, 0x6E, 0x66, 0x6F, 0x72, 0x6D, 0x61, 0x74, 0x69, 0x6B, 0x20, 0x47, 0x6D, 0x62, 0x48, 0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x44, 0x45, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x3E, 0x99, 0x74, 0x89, 0xFE, 0xAF, 0x47, 0x8C, 0x2F, 0x23, 0xB4, 0x0A, 0x2F, 0xF1, 0x5F, 0xB4, 0xE5, 0x2E, 0x23, 0x4D, 0xE6, 0x8A, 0x8E, 0x8A, 0xB8, 0xE6, 0xA9, 0x3A, 0x89, 0xD2, 0x2A, 0xE9, 0xF0, 0xE9, 0xCB, 0xC5, 0x52, 0x52, 0xD6, 0xF6, 0x49, 0x44, 0x40, 0x13, 0x9A, 0x74, 0x65, 0x04, 0x5C, 0xE9, 0x27, 0x64, 0xB9, 0x25, 0x0C, 0x71, 0x39, 0xC5, 0x3F, 0x75, 0x3D, 0x3A, 0x02, 0xD4, 0xA3, 0x66, 0x30, 0x64, 0x30, 0x12, 0x06, 0x03, 0x55, 0x1D, 0x13, 0x01, 0x01, 0xFF, 0x04, 0x08, 0x30, 0x06, 0x01, 0x01, 0xFF, 0x02, 0x01, 0x00, 0x30, 0x0E, 0x06, 0x03, 0x55, 0x1D, 0x0F, 0x01, 0x01, 0xFF, 0x04, 0x04, 0x03, 0x02, 0x01, 0xC6, 0x30, 0x1F, 0x06, 0x03, 0x55, 0x1D, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0xC2, 0x4C, 0xEE, 0x7D, 0xBC, 0x5E, 0xA6, 0x68, 0x45, 0xDE, 0x39, 0xF0, 0xDF, 0x75, 0x78, 0xF0, 0x61, 0x5F, 0x95, 0xEB, 0x30, 0x1D, 0x06, 0x03, 0x55, 0x1D, 0x0E, 0x04, 0x16, 0x04, 0x14, 0x13, 0x8F, 0x91, 0x6E, 0x7C, 0x82, 0x5D, 0x4C, 0x8C, 0xAA, 0x26, 0x7B, 0xCC, 0x23, 0x6D, 0xF1, 0x6B, 0x10, 0xE7, 0x2F, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x03, 0x48, 0x00, 0x30, 0x45, 0x02, 0x20, 0x60, 0x04, 0xB4, 0x06, 0x93, 0x07, 0xC4, 0x78, 0xE4, 0xB2, 0x94, 0x0A, 0xEE, 0x11, 0x91, 0xA8, 0x04, 0xAC, 0xFF, 0xEC, 0x00, 0xDA, 0xBD, 0xA9, 0x62, 0x5B, 0x37, 0xD1, 0x29, 0x94, 0xEB, 0x75, 0x02, 0x21, 0x00, 0xD7, 0x7B, 0x74, 0x5F, 0x15, 0x1D, 0xEA, 0xC3, 0x2A, 0x2D, 0xD0, 0x92, 0x07, 0x9D, 0x5F, 0xB8, 0xE8, 0xE9, 0x78, 0x42, 0xB2, 0x1D, 0xDF, 0xAD, 0x30, 0x4A, 0x7B, 0x4E, 0xFE, 0xDB, 0xE3, 0x9F, 0x0E, 0x80, 0x33, 0x08, 0x20, 0x1E, 0x43, 0x08, 0x20, 0x18, 0x9A, 0x00, 0x30, 0x20, 0x10, 0x20, 0x20, 0x90, 0x08, 0x94, 0x68, 0x31, 0x5A, 0xF1, 0x31, 0xED, 0x13, 0x00, 0xA0, 0x60, 0x82, 0xA8, 0x64, 0x8C, 0xE3, 0xD0, 0x40, 0x30, 0x23, 0x04, 0x33, 0x11, 0x33, 0x01, 0x10, 0x60, 0x35, 0x50, 0x40, 0x30, 0xC0, 0xA4, 0xD4, 0xF5, 0x26, 0xF6, 0xF7, 0x44, 0x34, 0x15, 0xF5, 0x83, 0x11, 0xF3, 0x01, 0xD0, 0x60, 0x35, 0x50, 0x40, 0xA0, 0xC1, 0x65, 0x66, 0x56, 0x37, 0x46, 0xF7, 0x22, 0x04, 0x96, 0xE6, 0x66, 0xF7, 0x26, 0xD6, 0x17, 0x46, 0x96, 0xB2, 0x04, 0x76, 0xD6, 0x24, 0x83, 0x10, 0xB3, 0x00, 0x90, 0x60, 0x35, 0x50, 0x40, 0x61, 0x30, 0x24, 0x44, 0x53, 0x02, 0x01, 0x70, 0xD3, 0x23, 0x23, 0x03, 0x13, 0x13, 0x23, 0x13, 0x43, 0x53, 0x73, 0x03, 0x95, 0xA1, 0x80, 0xF3, 0x23, 0x03, 0x53, 0x23, 0x03, 0x13, 0x03, 0x53, 0x13, 0x43, 0x53, 0x73, 0x03, 0x95, 0xA3, 0x04, 0x13, 0x11, 0x13, 0x00, 0xF0, 0x60, 0x35, 0x50, 0x40, 0x30, 0xC0, 0x84, 0xD4, 0xF5, 0x37, 0x56, 0x23, 0x15, 0xF5, 0x83, 0x11, 0xF3, 0x01, 0xD0, 0x60, 0x35, 0x50, 0x40, 0xA0, 0xC1, 0x65, 0x66, 0x56, 0x37, 0x46, 0xF7, 0x22, 0x04, 0x96, 0xE6, 0x66, 0xF7, 0x26, 0xD6, 0x17, 0x46, 0x96, 0xB2, 0x04, 0x76, 0xD6, 0x24, 0x83, 0x10, 0xB3, 0x00, 0x90, 0x60, 0x35, 0x50, 0x40, 0x61, 0x30, 0x24, 0x44, 0x53, 0x05, 0x93, 0x01, 0x30,0x60, 0x72, 0xA8, 0x64, 0x8C, 0xE3, 0xD0, 0x20, 0x10, 0x60, 0x82, 0xA8, 0x64, 0x8C, 0xE3, 0xD0, 0x30, 0x10, 0x70, 0x34, 0x20, 0x00, 0x4E, 0x6D, 0x6C, 0xDF, 0x64, 0x50, 0x0D, 0x56, 0x4E, 0x33, 0xE6, 0x6F, 0x8D, 0xC2, 0xC4, 0xB4, 0x04, 0x55, 0xB2, 0xD7, 0x3C, 0x68, 0x0A, 0x9A, 0x11, 0x6A,0xBE, 0x7A, 0x50, 0x60, 0x71, 0x9F, 0xE0, 0x21, 0xA0, 0xB4, 0xE7, 0xE7, 0x93, 0x52, 0x49, 0xDD, 0x19, 0x41, 0x12, 0x1C, 0x75, 0xB8, 0xCF, 0xBA, 0xC9, 0x67, 0x0E, 0xA9, 0x45, 0xC9, 0x73, 0x12, 0x26, 0x94, 0xEB, 0x73, 0x2B, 0x31, 0x4A, 0x36, 0x63, 0x06, 0x43, 0x01, 0x20, 0x60, 0x35, 0x51, 0xD1, 0x30, 0x10, 0x1F, 0xF0, 0x40, 0x83, 0x00, 0x60, 0x10, 0x1F, 0xF0, 0x20, 0x10, 0x13, 0x00, 0xE0, 0x60, 0x35, 0x51, 0xD0, 0xF0, 0x10, 0x1F, 0xF0, 0x40, 0x40, 0x30, 0x20, 0x10, 0x63, 0x01, 0xF0, 0x60, 0x35, 0x51, 0xD2, 0x30, 0x41, 0x83, 0x01, 0x68, 0x01, 0x41, 0x17, 0xE9, 0xEF, 0x70, 0x14, 0x26, 0xCC, 0x39, 0xA9, 0xF8, 0xAB, 0xF3, 0x62, 0x99, 0xDC, 0x0A, 0xDF, 0x1A, 0x0F, 0x33, 0x01, 0xD0, 0x60, 0x35, 0x51, 0xD0, 0xE0, 0x41, 0x60, 0x41, 0x4C, 0x24, 0xCE, 0xE7, 0xDB, 0xC5, 0xEA, 0x66, 0x84, 0x5D, 0xE3, 0x9F, 0x0D, 0xF7, 0x57, 0x8F, 0x06, 0x15, 0xF9, 0x5E, 0xB3, 0x00, 0xA0, 0x60, 0x82, 0xA8, 0x64, 0x8C, 0xE3, 0xD0, 0x40, 0x30, 0x20, 0x34, 0x90, 0x03, 0x04, 0x60, 0x22, 0x10, 0x08, 0xDD, 0xF4, 0x09, 0xD6, 0xEF, 0x97, 0x48, 0xFB, 0xCA, 0x83, 0x49, 0xB8, 0xBA, 0x75, 0xDB, 0xA3, 0x48, 0x9F, 0x93, 0xC5, 0xD4, 0xE9, 0x24, 0x73, 0xA1, 0xAE, 0xD8, 0x2A, 0xD3, 0xD2, 0x25, 0x70, 0x22, 0x10, 0x0A, 0x16, 0x94, 0xE9, 0x7A, 0xEE, 0x13, 0x0C, 0x9F, 0x2C, 0x73, 0xF1, 0x42, 0xFE, 0x6A, 0xC5, 0xAB, 0x4D, 0x60, 0x78, 0x66, 0x42, 0xFD, 0x96, 0x3F, 0x1D, 0x29, 0x9C, 0x6F, 0x33, 0xF9, 0xB6, 0x42, 0x00};
        memcpy(newArray + 20, additionalBytes, sizeof(additionalBytes));
        stream1.data  = newArray;
        pos1  = 1428;
}

}

if(pos1 == 44){
uint8_t anotherSequence[] ={0x89, 0x5A, 0x1D, 0x1D, 0x1C, 0x0E, 0x8B, 0xCB, 0xDD, 0xDD, 0xDD, 0xCB, 0x9D, 0xCC, 0xCB, 0x9B, 0xDC, 0x99, 0xCB, 0xD5, 0x14, 0x8B, 0xD8, 0xD8, 0x5B, 0x9B, 0xDB, 0x9A, 0x58, 0xD8, 0x5B, 0x0B, 0x59, 0x5E, 0x1A, 0x4B, 0xD0, 0xD5, 0xA1, 0xD1, 0xD1, 0xC0, 0xE8, 0xBC, 0xBD, 0xDD, 0xDD, 0xDC, 0xB9, 0xDC, 0xCC, 0xB9, 0xBD, 0xC9, 0x9C, 0xBC, 0xC8, 0xC0, 0xC0, 0xC4, 0xBC, 0xC0, 0xD0, 0xBD, 0xE1, 0xB5, 0xB1, 0x91, 0xCD, 0xA5, 0x9C, 0xB5, 0xB5, 0xBD, 0xC9, 0x94, 0x8D, 0x95, 0x8D, 0x91, 0xCD, 0x84, 0xB5, 0xCD, 0xA1, 0x84, 0xC8, 0xD4, 0xD9, 0x10, 0x41, 0x1A, 0x4A, 0x21, 0x81, 0x81, 0x88, 0x12, 0xB4, 0x3A, 0x3A, 0x38, 0x1D, 0x17, 0x97, 0xBB, 0xBB, 0xBB, 0x97, 0x3B, 0x99, 0x97, 0x37, 0xB9, 0x33, 0x97, 0xAA, 0x29, 0x17, 0xB1, 0xB0, 0xB7, 0x37, 0xB7, 0x34, 0xB1, 0xB0, 0xB6, 0x16, 0xB2, 0xBC, 0x34, 0x97, 0xA4, 0x29, 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x77, 0x77, 0x77, 0x2E, 0x77, 0x33, 0x2E, 0x6F, 0x72, 0x67, 0x2F, 0x32, 0x30, 0x30, 0x31, 0x2F, 0x30, 0x34, 0x2F, 0x78, 0x6D, 0x6C, 0x65, 0x6E, 0x63, 0x23, 0x73, 0x68, 0x61, 0x32, 0x35, 0x36, 0x42, 0x08, 0x62, 0x3E, 0x6F, 0x57, 0x49, 0x95, 0x59, 0x9D, 0xF3, 0x90, 0x48, 0xFF, 0xA8, 0x5B, 0x3f, 0xB1, 0xB9, 0xA4, 0x34, 0x62, 0x93, 0x25, 0x7D, 0xCC, 0xBB, 0xDE, 0x94, 0xD8, 0x4E, 0x3B, 0x17, 0xC1, 0x28, 0x1C, 0x4E, 0x78, 0x1F, 0x7F, 0xE4, 0x47, 0x49, 0x07, 0xEA, 0x12, 0x56, 0x74, 0xBF, 0xB9, 0x87, 0xB0, 0xAD, 0x00, 0x4E, 0xA8, 0xA2, 0xA1, 0x72, 0xE8, 0x8E, 0xCE, 0xD6, 0xE8, 0x4B, 0xB0, 0x97, 0x15, 0x16, 0xBD, 0x54, 0x4F, 0xA9, 0x82, 0xB1, 0x88, 0xD8, 0x48, 0xF7, 0x0B, 0x22, 0xBE, 0x0D, 0x9C, 0x72, 0x76, 0x61, 0x2C, 0x99, 0xF2, 0x91, 0x9C, 0x86, 0x23, 0x7E, 0xA2, 0x82, 0x45, 0x74, 0x88, 0x00, 0x07};
 size_t newSize = 319;
 stream1.data[6] = 0x01;
  stream1.data[7] = 0x37;
valueAtIndex = stream1.data[19];
    // Allocate memory for the new array
    uint8_t* newArray = (uint8_t*)malloc(newSize);
   memcpy(newArray, stream1.data, 20);
   memcpy(newArray + 20, anotherSequence, sizeof(anotherSequence));

    // Extract the last 13 bytes from the sequence
    uint8_t Sequence[23];
    //int lastIndex = sizeof(stream1.data) - sizeof(Sequence);
//      printf(lastIndex);
    for (int i = 0; i < sizeof(Sequence); ++i) {
        Sequence[i] = stream1.data[21 + i];
    }
   memcpy(newArray + 296, Sequence, sizeof(Sequence));

        stream1.data  = newArray;
        pos1  = 319;
}
if(pos1 == 22){
stream1.data[19] += 1;
}
  if (send(evse_socket, stream1.data, pos1, 0) == -1) {
        perror("Error sending data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }


  
   if(pos1 == 1428){
	uint8_t anotherSequence[] = {
    0x09, 0xD6, 0xEF, 0x97, 0x48, 0xFB, 0xCA, 0x83, 0x49, 0xB8, 0xBA, 0x75, 0xDB, 0xA3, 0x48, 0x9F,
    0x93, 0xC5, 0xD4, 0xE9, 0x24, 0x73, 0xA1, 0xAE, 0xD8, 0x2A, 0xD3, 0xD2, 0x25, 0x70, 0x22, 0x10,
    0x0A, 0x16, 0x94, 0xE9, 0x7A, 0xEE, 0x13, 0x0C, 0x9F, 0x2C, 0x73, 0xF1, 0x42, 0xFE, 0x6A, 0xC5,
    0xAB, 0x4D, 0x60, 0x78, 0x66, 0x42, 0xFD, 0x96, 0x3F, 0x1D, 0x29, 0x9C, 0x6F, 0x33, 0xF9, 0xB6,
    0x42, 0x00};
  if (send(evse_socket,  anotherSequence, 66, 0) == -1) {
        perror("Error sending data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
}
    // Receive response from the EVSE
    ssize_t bytes_received = recv(evse_socket, stream2.data, BUFFER_SIZE, 0);
    if (bytes_received == -1) {
        perror("Error receiving data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
         valueAtIndex19 = stream2.data[19];
    // Deserialize the response from the server
    errn = deserialize2Stream2EXI(&stream2, exiOut, bytes_received);
}
    return errn;
}
static int startcharge(struct iso2EXIDocument* exiIn, struct iso2EXIDocument* exiOut,int use1EHeader, int ChargingLevel){
        int errn;
	int i = 0;
	int c = 0;
	int a = 1;
    bitstream_t stream1;
    bitstream_t stream2;
    size_t pos1;
    size_t pos2;

    stream1.size = BUFFER_SIZE;
    stream1.data = buffer1;
    stream1.pos = &pos1;

    stream2.size = BUFFER_SIZE;
    stream2.data = buffer2;
    stream2.pos = &pos2;
    int shmid;
    key_t key = SHM_KEY;
    int *shm_ptr;

// Create a shared memory segment
    if ((shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    // Attach to the shared memory segment
    if ((shm_ptr = shmat(shmid, NULL, 0)) == (int *) -1) {
        perror("shmat");
        exit(1);
    }
printf("Charginglevel: %d\n", ChargingLevel);
errn = serialize2EXI2Stream(exiIn, &stream1, use1EHeader);
valueAtIndex19 -= 1;
stream1.data[19] =  valueAtIndex19;
 size_t newSize = 54;
 stream1.data[7] = 0x2E;
 valueAtIndex22 = 0x00;
 uint8_t anotherSequence[] = {0xD0,0x00, 0x00, 0x02, 0x86, 0x00, 0x20, 0x28, 0x80, 0x0A, 0x02, 0x86,0x00, 0xA0, 0x61, 0x43, 0x78, 0x08, 0x00, 0x06, 0x08, 0x3F, 0xFF, 0xC0, 0x40, 0xC1, 0x07, 0xFF, 0xF8, 0x08, 0x28, 0x80, 0x06, 0x00};
   // Allocate memory for the new array
   uint8_t* newArray = (uint8_t*)malloc(newSize);
   memcpy(newArray, stream1.data,20);
   memcpy(newArray + 20, anotherSequence, sizeof(anotherSequence));
        stream1.data  = newArray;
        pos1  = 54;
    if (send(evse_socket, stream1.data, pos1, 0) == -1) {
        perror("Error sending data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
    // Receive response from the EVSE
    ssize_t bytes_received = recv(evse_socket, stream2.data, BUFFER_SIZE, 0);
    if (bytes_received == -1) {
        perror("Error receiving data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
int charge = ChargingLevel + 1;
while ((i < ChargingLevel ) || (i >= ChargingLevel  && a != 0)) {
if(a % 3 == 0 && a != 0){
 size_t newSize = 331;
stream1.data[6] = 0x01;
 stream1.data[7] = 0x43;
stream1.data[19] = valueAtIndex;
 uint8_t anotherSequence[] = {0x89, 0x5A, 0x1D, 0x1D, 0x1C, 0x0E, 0x8B, 0xCB, 0xDD, 0xDD, 0xDD, 0xCB, 0x9D, 0xCC, 0xCB, 0x9B, 0xDC, 0x99, 0xCB, 0xD5, 0x14, 0x8B, 0xD8, 0xD8, 0x5B, 0x9B, 0xDB, 0x9A, 0x58, 0xD8, 0x5B, 0x0B, 0x59, 0x5E, 0x1A, 0x4B, 0xD0, 0xD5, 0xA1, 0xD1, 0xD1, 0xC0, 0xE8, 0xBC, 0xBD, 0xDD, 0xDD, 0xDC, 0xB9, 0xDC, 0xCC, 0xB9, 0xBD, 0xC9, 0x9C, 0xBC, 0xC8, 0xC0, 0xC0, 0xC4, 0xBC, 0xC0, 0xD0, 0xBD, 0xE1, 0xB5, 0xB1, 0x91, 0xCD, 0xA5, 0x9C, 0xB5, 0xB5, 0xBD, 0xC9, 0x94, 0x8D, 0x95, 0x8D, 0x91, 0xCD, 0x84, 0xB5, 0xCD, 0xA1, 0x84, 0xC8, 0xD4, 0xD9, 0x10, 0x41, 0x1A, 0x4A, 0x21, 0x81, 0x81, 0x88, 0x12, 0xB4, 0x3A, 0x3A, 0x38, 0x1D, 0x17, 0x97, 0xBB, 0xBB, 0xBB, 0x97, 0x3B, 0x99, 0x97, 0x37, 0xB9, 0x33, 0x97, 0xAA, 0x29, 0x17, 0xB1, 0xB0, 0xB7, 0x37, 0xB7, 0x34, 0xB1, 0xB0, 0xB6, 0x16, 0xB2, 0xBC, 0x34, 0x97, 0xA4, 0x29, 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x77, 0x77, 0x77, 0x2E, 0x77, 0x33, 0x2E, 0x6F, 0x72, 0x67, 0x2F, 0x32, 0x30, 0x30, 0x31, 0x2F, 0x30, 0x34, 0x2F, 0x78, 0x6D, 0x6C, 0x65, 0x6E, 0x63, 0x23, 0x73, 0x68, 0x61, 0x32, 0x35, 0x36, 0x42, 0x08, 0xF2, 0x33, 0xF5, 0x7F, 0xC7, 0xAC, 0x59, 0x8B, 0x1D, 0x1F, 0x9B, 0xB1, 0xAD, 0xC0, 0xF6, 0x08, 0x10, 0x47, 0x02, 0x19, 0x00, 0xA5, 0xEB, 0xE8, 0x5B, 0x23, 0x47, 0xF0, 0x08, 0x13, 0x60, 0xB1, 0x28, 0x09, 0x26, 0x45, 0x9D, 0x5E, 0x00, 0x36, 0xE5, 0x1D, 0xB5, 0x9D, 0x54, 0xAE, 0xCA, 0x42, 0x10, 0x50, 0xEA, 0xA0, 0x2A, 0x6F, 0x13, 0xFD, 0x95, 0xCE, 0xCC, 0x50, 0x10, 0x57, 0xAD, 0x50, 0xAD, 0x20, 0x99, 0x21, 0xD5, 0x5E, 0xA4, 0xB0, 0x04, 0x01, 0x75, 0x4D, 0xD3, 0xE9, 0xD4, 0x31, 0x75, 0x35, 0x43, 0xD6, 0x80, 0x5A, 0x21, 0x7D, 0xBE, 0x03, 0x16, 0x18, 0x45, 0xB0, 0x23, 0xE2, 0xC1, 0x08, 0x3C, 0x07, 0x49, 0x44, 0x30, 0x30, 0x31, 0x02, 0x1C, 0x92, 0x45, 0xE3, 0xAB, 0xC6, 0x11, 0x1E, 0x80, 0x04, 0x02, 0xCC, 0x0E, 0x0C, 0x4D, 0x53, 0x59, 0x5D, 0x19, 0x5C, 0x80, 0x0E, 0x44, 0x15, 0xC4, 0xDD, 0x58, 0x48, 0x00};
   uint8_t* newArray = (uint8_t*)malloc(newSize);
   memcpy(newArray, stream1.data,20);
   memcpy(newArray + 20, anotherSequence, sizeof(anotherSequence));
        //stream1.data  = newArray;
        pos1  = 331;
    if (send(evse_socket, newArray, pos1, 0) == -1) {
        perror("Error sending data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
    ssize_t bytes_received = recv(evse_socket, stream2.data, BUFFER_SIZE, 0);
    if (bytes_received == -1) {
        perror("Error receiving data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
a = 0;
}else{
if(c == 0){
stream1.data[19] =  valueAtIndex19;
 size_t newSize = 52;
// stream1.data[6] = 0x01;
 stream1.data[7] = 0x2C;
stream1.data[6] = 0x00;
 uint8_t anotherSequence[] = {0xD0, 0x00, 0x00, 0x82, 0x86, 0x00, 0x20, 0x28, 0x80, 0x0A, 0x02, 0x86, 0x00, 0xA0, 0x61, 0x43, 0x78, 0x08, 0x00, 0x02, 0x08, 0x25, 0x42, 0x40, 0x41, 0x05, 0x88, 0x38, 0x28, 0x80, 0x06, 0x00};
   uint8_t* newArray = (uint8_t*)malloc(newSize);
   memcpy(newArray, stream1.data,20);
   memcpy(newArray + 20, anotherSequence, sizeof(anotherSequence));
        stream1.data  = newArray;
        pos1  = 52;
    stream1.data[22] =  valueAtIndex22;
    if (send(evse_socket, stream1.data, pos1, 0) == -1) {
        perror("Error sending data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
    // Receive response from the EVSE
    ssize_t bytes_received = recv(evse_socket, stream2.data, BUFFER_SIZE, 0);
    if (bytes_received == -1) {
        perror("Error receiving data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
if(i < ChargingLevel - 1){
    valueAtIndex22 += 1;
    c = 1;
}
}else{
stream1.data[19] =  valueAtIndex19;
 size_t newSize = 52;
 stream1.data[7] = 0x2C;
stream1.data[6] = 0x00;
 uint8_t anotherSequence[] = {0xD0,0x00, 0x01, 0x02, 0x86, 0x00, 0x20, 0x28, 0x80, 0x0A, 0x02, 0x86,0x00, 0xA0, 0x61, 0x43, 0x78, 0x08, 0x00, 0x06, 0x08, 0x00};
   //Allocate memory for the new array
  uint8_t* newArray = (uint8_t*)malloc(newSize);
   memcpy(newArray, stream1.data,20);
   memcpy(newArray + 20, anotherSequence, sizeof(anotherSequence));
        stream1.data  = newArray;
stream1.data[22] =  valueAtIndex22;
      pos1  = 52;
    if (send(evse_socket, stream1.data, pos1, 0) == -1) {
        perror("Error sending data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
    // Receive response from the EVSE
    ssize_t bytes_received = recv(evse_socket, stream2.data, BUFFER_SIZE, 0);
    if (bytes_received == -1) {
        perror("Error receiving data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
if(i < ChargingLevel - 1){
c = 0;
}
}
if(i < ChargingLevel){
i += 1;
printf("Battery level");
printf("i = %d\n", i); // Print the value of i
printf("a = %d\n", a); // Print the value of i
}
a += 1;
printf("a = %d\n", a); // Print the value of i

}
}
    // Deserialize the response from the server
    errn = 0;
if (errn != 0) {
    // Handle decoding error
    printf("Error decoding EVSE payload\n");
    return errn;
}
 // Detach from the shared memory segment
    if (shmdt(shm_ptr) == -1) {
        perror("shmdt");
        exit(1);
    }
    return errn;

}



static int precharge(struct iso2EXIDocument* exiIn, struct iso2EXIDocument* exiOut,int use1EHeader){
    int errn;
    int i = 0;
    bitstream_t stream1;
    bitstream_t stream2;

    size_t pos1;
    size_t pos2;

    stream1.size = BUFFER_SIZE;
    stream1.data = buffer1;
    stream1.pos = &pos1;

    stream2.size = BUFFER_SIZE;
    stream2.data = buffer2;
    stream2.pos = &pos2;

errn = serialize2EXI2Stream(exiIn, &stream1, use1EHeader);
valueAtIndex19 +=1;
stream1.data[19] =  valueAtIndex19;
 size_t newSize = 31;
 stream1.data[7] = 0x17;
 uint8_t anotherSequence[] = {0x70, 0x00, 0x00, 0x02, 0x88, 0x00, 0x60, 0x81, 0x80, 0x08, 0x00};
   // Allocate memory for the new array
   uint8_t* newArray = (uint8_t*)malloc(newSize);
   memcpy(newArray, stream1.data,20);
   memcpy(newArray + 20, anotherSequence, sizeof(anotherSequence));
        stream1.data  = newArray;
        pos1  = 31;
while(i <= 300){
    if (send(evse_socket, stream1.data, pos1, 0) == -1) {
        perror("Error sending data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
    // Receive response from the EVSE
    ssize_t bytes_received = recv(evse_socket, stream2.data, BUFFER_SIZE, 0);
    if (bytes_received == -1) {
        perror("Error receiving data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
    // Deserialize the response from the server
    errn = 0;
if (errn != 0) {
    // Handle decoding error
    printf("Error decoding EVSE payload\n");
    return errn;
}
i +=20;
}
    return errn;
}



static int cablecheck(struct iso2EXIDocument* exiIn, struct iso2EXIDocument* exiOut,int use1EHeader){
        int errn;

    bitstream_t stream1;
    bitstream_t stream2;

    size_t pos1;
    size_t pos2;

    stream1.size = BUFFER_SIZE;
    stream1.data = buffer1;
    stream1.pos = &pos1;

    stream2.size = BUFFER_SIZE;
    stream2.data = buffer2;
    stream2.pos = &pos2;
errn = serialize2EXI2Stream(exiIn, &stream1, use1EHeader);
stream1.data[19] =  valueAtIndex19;
 size_t newSize = 24;
 stream1.data[7] = 0x10;
 uint8_t anotherSequence[] = {0x30, 0x00, 0x00, 0x00 };
   // Allocate memory for the new array
   uint8_t* newArray = (uint8_t*)malloc(newSize);
   memcpy(newArray, stream1.data,20);
   memcpy(newArray + 20, anotherSequence, sizeof(anotherSequence));
        stream1.data  = newArray;
        pos1  = 24;
    if (send(evse_socket, stream1.data, pos1, 0) == -1) {
        perror("Error sending data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
    // Receive response from the EVSE
    ssize_t bytes_received = recv(evse_socket, stream2.data, BUFFER_SIZE, 0);
    if (bytes_received == -1) {
        perror("Error receiving data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
    // Print the received payload
    valueAtIndex19 = stream2.data[19];
    // Deserialize the response from the server
    errn = 0;
if (errn != 0) {
    // Handle decoding error
    printf("Error decoding EVSE payload\n");
    return errn;
}
    return errn;
}

static int chargingpar(struct iso2EXIDocument* exiIn, struct iso2EXIDocument* exiOut,int use1EHeader){
        int errn;

    bitstream_t stream1;
    bitstream_t stream2;

    size_t pos1;
    size_t pos2;

    stream1.size = BUFFER_SIZE;
    stream1.data = buffer1;
    stream1.pos = &pos1;

    stream2.size = BUFFER_SIZE;
    stream2.data = buffer2;
    stream2.pos = &pos2;
errn = serialize2EXI2Stream(exiIn, &stream1, use1EHeader);
stream1.data[19] =  valueAtIndex19;
 size_t newSize = 48;
 stream1.data[7] = 0x28;
 uint8_t anotherSequence[] = {0x90,0x18, 0x19, 0x40, 0x00, 0x00, 0x14, 0x30, 0x05, 0x03, 0x0A, 0x1B, 0xC0, 0x40, 0xA2, 0x00, 0x28, 0x0A, 0x30, 0x00, 0x80, 0xA3, 0x00, 0x08, 0x31, 0x85, 0x00, 0x00};
   // Allocate memory for the new array
   uint8_t* newArray = (uint8_t*)malloc(newSize);
   memcpy(newArray, stream1.data,20);
   memcpy(newArray + 20, anotherSequence, sizeof(anotherSequence));
        stream1.data  = newArray;
        pos1  = 48;
    printBinaryArray(stream1.data, pos1);
    if (send(evse_socket, stream1.data, pos1, 0) == -1) {
        perror("Error sending data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
    // Receive response from the EVSE
    ssize_t bytes_received = recv(evse_socket, stream2.data, BUFFER_SIZE, 0);
    if (bytes_received == -1) {
        perror("Error receiving data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
    // Print the received payload
    // Deserialize the response from the server
    errn = 0;
if (errn != 0) {
    // Handle decoding error
    printf("Error decoding EVSE payload\n");
    return errn;
}
    return errn;


}


static int power(struct iso2EXIDocument* exiIn, struct iso2EXIDocument* exiOut,int use1EHeader){
  	int errn;

    bitstream_t stream1;
    bitstream_t stream2;

    size_t pos1;
    size_t pos2;

    stream1.size = BUFFER_SIZE;
    stream1.data = buffer1;
    stream1.pos = &pos1;

    stream2.size = BUFFER_SIZE;
    stream2.data = buffer2;
    stream2.pos = &pos2;
errn = serialize2EXI2Stream(exiIn, &stream1, use1EHeader);
stream1.data[19] =  valueAtIndex19;
 size_t newSize = 54;
 stream1.data[7] = 0x2E;
 uint8_t anotherSequence[] = {0x50, 0x00, 0x20, 0x00, 0x03, 0x0A, 0x00, 0x22, 0x11, 0x43, 0x81, 0x85, 0x00, 0xA1, 0x0D, 0xAD, 0x40, 0x30, 0xC2, 0x80, 0x78, 0x86, 0x54, 0x18, 0x20, 0x61, 0x40, 0x50, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00};
   // Allocate memory for the new array
   uint8_t* newArray = (uint8_t*)malloc(newSize);
   memcpy(newArray, stream1.data,20);
   memcpy(newArray + 20, anotherSequence, sizeof(anotherSequence));
        stream1.data  = newArray;
        pos1  = 54;
    printBinaryArray(stream1.data, pos1);
    if (send(evse_socket, stream1.data, pos1, 0) == -1) {
        perror("Error sending data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
    // Receive response from the EVSE
    ssize_t bytes_received = recv(evse_socket, stream2.data, BUFFER_SIZE, 0);
    if (bytes_received == -1) {
        perror("Error receiving data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
    // Deserialize the response from the server
    errn = 0;
if (errn != 0) {
    // Handle decoding error
    printf("Error decoding EVSE payload\n");
    return errn;
}
    return errn;


}
static int request_response(struct iso2EXIDocument* exiIn, struct iso2EXIDocument* exiOut,int use1EHeader) {
  int errn;

    bitstream_t stream1;
    bitstream_t stream2;

    size_t pos1;
    size_t pos2;

    stream1.size = BUFFER_SIZE;
    stream1.data = buffer1;
    stream1.pos = &pos1;

    stream2.size = BUFFER_SIZE;
    stream2.data = buffer2;
    stream2.pos = &pos2;
print_iso2EXIDocument(exiIn);
    /* EV side */
    errn = serialize2EXI2Stream(exiIn, &stream1, use1EHeader);
const uint8_t targetSequence2[] = {0x00, 0x09, 0x10, 0x18, 0x08, 0x00, 0x00, 0x00};
size_t position2 = findByteSequence(stream1.data, pos1, targetSequence2, sizeof(targetSequence2));
if (position2 < pos1) {
    stream1.data[position2 + 1] = 0x11;
    stream1.data[position2 + 2] = 0xD0;
} 
if(pos1 == 28){
stream1.data[7] = 0x15;
}
    if (send(evse_socket, stream1.data, pos1, 0) == -1) {
        perror("Error sending data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
    // Receive response from the EVSE
    ssize_t bytes_received = recv(evse_socket, stream2.data, BUFFER_SIZE, 0);
    if (bytes_received == -1) {
        perror("Error receiving data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
	 valueAtIndex19 = stream2.data[19];
    // Deserialize the response from the server
    errn = deserialize2Stream2EXI(&stream2, exiOut, bytes_received);
if (errn != 0) {
    // Handle decoding error
    printf("Error decoding EVSE payload\n");
    return errn;
}
    return errn;

}

int appHandshake() {
	V2GTP_VERSION_INV = 0xFE;
    bitstream_t stream1;
    size_t pos1 = V2GTP_HEADER_LENGTH; /* v2gtp header */
    struct appHandEXIDocument handshake;
    int errn = 0;

    char* ns0 = "urn:din:70121:2012:MsgDef";
    char* ns1 = "urn:iso:15118:2:2013:MsgDef";

    stream1.size = BUFFER_SIZE;
    stream1.data = buffer1;
    stream1.pos = &pos1;

    init_appHandEXIDocument(&handshake);

 
    printf("EV side: setup data for the supported application handshake request message\n");

/* set up ISO/IEC 15118 Version 1.0 information */
handshake.supportedAppProtocolReq_isUsed = 1u;
handshake.supportedAppProtocolReq.AppProtocol.arrayLen = 2; /* we have only two protocols implemented */

// AppProtocol (0)
handshake.supportedAppProtocolReq.AppProtocol.array[0].ProtocolNamespace.charactersLen =
    writeStringToEXIString(ns0, handshake.supportedAppProtocolReq.AppProtocol.array[0].ProtocolNamespace.characters);
handshake.supportedAppProtocolReq.AppProtocol.array[0].SchemaID = 0;
handshake.supportedAppProtocolReq.AppProtocol.array[0].VersionNumberMajor = 2;
handshake.supportedAppProtocolReq.AppProtocol.array[0].VersionNumberMinor = 0;
handshake.supportedAppProtocolReq.AppProtocol.array[0].Priority = 2;

// AppProtocol (1)
handshake.supportedAppProtocolReq.AppProtocol.array[1].ProtocolNamespace.charactersLen =
    writeStringToEXIString(ns1, handshake.supportedAppProtocolReq.AppProtocol.array[1].ProtocolNamespace.characters);
handshake.supportedAppProtocolReq.AppProtocol.array[1].SchemaID = 1;
handshake.supportedAppProtocolReq.AppProtocol.array[1].VersionNumberMajor = 2;
handshake.supportedAppProtocolReq.AppProtocol.array[1].VersionNumberMinor = 0;
handshake.supportedAppProtocolReq.AppProtocol.array[1].Priority = 1;
    // Create a TCP socket
evse_socket = socket(AF_INET6, SOCK_STREAM, 0);
if (evse_socket == -1) {
    error("Error creating socket");
}

// Initialize evse_addr
memset(&evse_addr, 0, sizeof(evse_addr));
evse_addr.sin6_family = AF_INET6;
evse_addr.sin6_port = htons(APP_HANDSHAKE_PORT);
evse_addr.sin6_flowinfo = 0; // Set to 0
evse_addr.sin6_scope_id = if_nametoindex("eth0"); // Specify the network interface
char ip_str[INET6_ADDRSTRLEN];
// Convert EVSE IP address to binary form
int conversion_result = inet_pton(AF_INET6, APP_IP, &evse_addr.sin6_addr);
if (conversion_result != 1) {
    perror("Error converting IP address");
    close(evse_socket);
    exit(EXIT_FAILURE);
}

// Print debugging info
printf("Attempting to connect to the EVSE at [%s]:%d\n", APP_IP, APP_HANDSHAKE_PORT);

// Attempt to connect
if (connect(evse_socket, (struct sockaddr*)&evse_addr, sizeof(evse_addr)) == -1) {
    perror("Error connecting to the EVSE");
    close(evse_socket);
    exit(EXIT_FAILURE);
}
    // Print the original IP address
    printf("%s:%d\n", APP_IP, APP_HANDSHAKE_PORT);
/* send app handshake request */
    if ((errn = encode_appHandExiDocument(&stream1, &handshake)) == 0) {
        printf("ENCODED!! ");

        if (write_v2gtpHeader(stream1.data, pos1 - V2GTP_HEADER_LENGTH, V2GTP_EXI_TYPE) == 0) {

    // Send the modified payload to the EVSE
    if (send(evse_socket, stream1.data, pos1, 0) == -1) {
        perror("Error sending data");
        close(evse_socket);
        exit(EXIT_FAILURE);
    }
// Wait for the response
ssize_t bytes_received = recv(evse_socket, buffer1, BUFFER_SIZE, 0);
if (bytes_received == -1) {
    perror("Error receiving data");
    close(evse_socket);
    exit(EXIT_FAILURE);
}            // Print the values in handshake after encoding
            printf("EV side: After encoding appHandEXIDocument\n");
            // Example: Print ProtocolNamespace for the first AppProtocol
            printf("ProtocolNamespace (AppProtocol 0): %s\n", handshake.supportedAppProtocolReq.AppProtocol.array[0].ProtocolNamespace.characters);
           
        }
    }

    if (errn != 0) {
        printf("appHandshake error %d \n", errn);
    }
    return errn;
  
}

#if DEPLOY_ISO2_CODEC == SUPPORT_YES
void printHexRepresentation(const char* str) {
    for (size_t i = 0; str[i] != '\0'; ++i) {
        printf("%02X ", (unsigned char)str[i]);
    }
}

static void printEVSEStatus2(struct iso2EVSEStatusType* status)
{
        printf("\tEVSEStatus:\n");
        printf("\t\tEVSENotification=%d\n", status->EVSENotification);
        printf("\t\tNotificationMaxDelay=%d\n", status->NotificationMaxDelay);
}
void printHex(const void* data, size_t size) {
    const unsigned char* ptr = (const unsigned char*)data;
    for (size_t i = 0; i < size; ++i) {
        printf("%02X ", ptr[i]);
    }
    printf("\n");
}

void printEXIInHex(struct iso2EXIDocument* exiIn) {
    printf("EXIIn structure in hexadecimal:\n");
    printf("V2G_Message_isUsed: %u\n", exiIn->V2G_Message_isUsed);
    printf("SessionID: ");
    printHex(exiIn->V2G_Message.Header.SessionID.bytes, exiIn->V2G_Message.Header.SessionID.bytesLen);
    // Print other fields as needed
}
static int charging2(int Charginglevel)
{
    int errn = 0;
    int i, j;
	struct iso2EXIDocument exiIn;
        struct iso2EXIDocument exiOut;

        struct iso2ServiceDetailResType serviceDetailRes;
        struct iso2PaymentServiceSelectionResType paymentServiceSelectionRes;
        struct iso2PaymentDetailsResType paymentDetailsRes;

    /* setup header information */
    init_iso2EXIDocument(&exiIn);
    exiIn.V2G_Message_isUsed = 1u;
    init_iso2MessageHeaderType(&exiIn.V2G_Message.Header);
    exiIn.V2G_Message.Header.SessionID.bytes[0] = 0; /* sessionID is always '0' at the beginning (the response contains the valid sessionID)*/
    exiIn.V2G_Message.Header.SessionID.bytes[1] = 0;
    exiIn.V2G_Message.Header.SessionID.bytes[2] = 0;
    exiIn.V2G_Message.Header.SessionID.bytes[3] = 0;
    exiIn.V2G_Message.Header.SessionID.bytes[4] = 0;
    exiIn.V2G_Message.Header.SessionID.bytes[5] = 0;
    exiIn.V2G_Message.Header.SessionID.bytes[6] = 0;
    exiIn.V2G_Message.Header.SessionID.bytes[7] = 0;
    exiIn.V2G_Message.Header.SessionID.bytesLen = 8;
//exiIn.V2G_Message.Header.SessionID.bytesLen = 16;
printf("Header Information:\n");
printf("V2G_Message_isUsed: %u\n", exiIn.V2G_Message_isUsed);
printf("| Session ID   ");
for (int i = 0; i < exiIn.V2G_Message.Header.SessionID.bytesLen; ++i) {
    printf("%02X", exiIn.V2G_Message.Header.SessionID.bytes[i]);
}
printf("\n");


    /************************
    * sessionSetup *
    ************************/
    init_iso2BodyType(&exiIn.V2G_Message.Body);
    exiIn.V2G_Message.Body.SessionSetupReq_isUsed = 1u;

     init_iso2SessionSetupReqType(&exiIn.V2G_Message.Body.SessionSetupReq);
     exiIn.V2G_Message.Body.SessionSetupReq.EVCCID.bytesLen = 6; // Set the length to 6 bytes
     exiIn.V2G_Message.Body.SessionSetupReq.EVCCID.bytes[0] = 0x02;
     exiIn.V2G_Message.Body.SessionSetupReq.EVCCID.bytes[1] = 0x00;
     exiIn.V2G_Message.Body.SessionSetupReq.EVCCID.bytes[2] = 0x00;
     exiIn.V2G_Message.Body.SessionSetupReq.EVCCID.bytes[3] = 0x00;
     exiIn.V2G_Message.Body.SessionSetupReq.EVCCID.bytes[4] = 0x00;
     exiIn.V2G_Message.Body.SessionSetupReq.EVCCID.bytes[5] = 0x01;
printf("| Session ID   ");
for (int i = 0; i < exiIn.V2G_Message.Body.SessionSetupReq.EVCCID.bytesLen; ++i) {
    printf("%02X", exiIn.V2G_Message.Body.SessionSetupReq.EVCCID.bytes[i]);
}
printf("\n");
printEXIInHex(&exiIn);
     printf("EV side: call EVSE sessionSetup");
	errn = request_response(&exiIn, &exiOut, 0);
			/* show results of EVSEs answer message */
			if(errn == 0) {
		/* check, if this is the right response message */
		if(exiOut.V2G_Message.Body.SessionSetupRes_isUsed) {
			/* show results of EVSEs answer message */
			printf("EV side: received response message from EVSE\n");
			printf("\tHeader SessionID=");
			printBinaryArray(exiOut.V2G_Message.Header.SessionID.bytes, exiOut.V2G_Message.Header.SessionID.bytesLen);
			printf("\tResponseCode=%d\n", exiOut.V2G_Message.Body.SessionSetupRes.ResponseCode);
			printf("\tEVSEID=%d\n",	exiOut.V2G_Message.Body.SessionSetupRes.EVSEID.characters[1]);
			printf("\tEVSETimeStamp=%li\n", (long int)exiOut.V2G_Message.Body.SessionSetupRes.EVSETimeStamp);
			printf("%zu\n", exiOut.V2G_Message.Header.SessionID.bytesLen);
			exiIn.V2G_Message.Header.SessionID = exiOut.V2G_Message.Header.SessionID;

		} else {
			errn = ERROR_UNEXPECTED_SESSION_SETUP_RESP_MESSAGE;
			return errn;
		}
	} else {
		return errn;
	}
	/*******************************************
	 * serviceDiscovery *
	 *******************************************/
	init_iso2BodyType(&exiIn.V2G_Message.Body);
	exiIn.V2G_Message.Body.ServiceDiscoveryReq_isUsed = 1u;
	init_iso2ServiceDiscoveryReqType(&exiIn.V2G_Message.Body.ServiceDiscoveryReq);
	exiIn.V2G_Message.Body.ServiceDiscoveryReq.SupportedServiceIDs_isUsed = 1u;
	exiIn.V2G_Message.Body.ServiceDiscoveryReq.SupportedServiceIDs.ServiceID.arrayLen = 1;
	printf("EV side: call EVSE serviceDiscovery");
	errn = request_response2(&exiIn, &exiOut, 0, 24);
	if(errn == 0) {
	printf("ServiceDiscoveryRes_isUsed: %d\n", exiOut.V2G_Message.Body.ServiceDiscoveryRes_isUsed);
			/* show results of EVSEs answer message */
			printf("EV side: received response message from EVSE\n");
			printf("\tHeader SessionID=");
			printBinaryArray(exiOut.V2G_Message.Header.SessionID.bytes, exiOut.V2G_Message.Header.SessionID.bytesLen);
			printf("\t ResponseCode=%d\n",  exiOut.V2G_Message.Body.ServiceDiscoveryRes.ResponseCode);

			printf("\t Service ResponseCode=%d\n",	exiOut.V2G_Message.Body.ServiceDiscoveryRes.ResponseCode);
	} else {
		return errn;
}
	/*******************************************
	 * ServicePaymentSelection *
	 *******************************************/
	init_iso2BodyType(&exiIn.V2G_Message.Body);
	exiIn.V2G_Message.Body.PaymentServiceSelectionReq_isUsed = 1u;

	init_iso2PaymentServiceSelectionReqType(&exiIn.V2G_Message.Body.PaymentServiceSelectionReq);

	exiIn.V2G_Message.Body.PaymentServiceSelectionReq.SelectedPaymentOption = iso2paymentOptionType_Contract;
	exiIn.V2G_Message.Body.PaymentServiceSelectionReq.SelectedVASList_isUsed = 0u;
	exiIn.V2G_Message.Body.PaymentServiceSelectionReq.SelectedEnergyTransferService.ServiceID = 1;
	printf("EV side: call EVSE ServicePaymentSelection \n");
	size_t sizeOfExiIn = sizeof(*(&exiIn));
	errn = request_response2(&exiIn, &exiOut, 0, 24);
	if(errn == 0) {
			paymentServiceSelectionRes = exiOut.V2G_Message.Body.PaymentServiceSelectionRes;
			/* show results of EVSEs answer message */
			printf("EV side: received response message from EVSE\n");
			if(exiOut.V2G_Message.Body.PaymentServiceSelectionRes.EVSEStatus_isUsed) {
				printf("\tHeader SessionID=");
				printBinaryArray(exiOut.V2G_Message.Header.SessionID.bytes, exiOut.V2G_Message.Header.SessionID.bytesLen);
			}
			printf("\t ResponseCode=%d\n",  paymentServiceSelectionRes.ResponseCode);
	} else {
		return errn;
	}



	/**********************************
	 * PaymentDetails *
	 **********************************/
	init_iso2BodyType(&exiIn.V2G_Message.Body);
	exiIn.V2G_Message.Body.PaymentDetailsReq_isUsed = 1u;

	init_iso2PaymentDetailsReqType(&exiIn.V2G_Message.Body.PaymentDetailsReq);

	exiIn.V2G_Message.Body.PaymentDetailsReq.eMAID.characters[0] = 68;  // ASCII value for 'D'
	exiIn.V2G_Message.Body.PaymentDetailsReq.eMAID.characters[1] = 69;  // ASCII value for 'E'
	exiIn.V2G_Message.Body.PaymentDetailsReq.eMAID.characters[2] = 56;  // ASCII value for '8'
	exiIn.V2G_Message.Body.PaymentDetailsReq.eMAID.characters[3] = 65;  // ASCII value for 'A'
	exiIn.V2G_Message.Body.PaymentDetailsReq.eMAID.characters[4] = 65;  // ASCII value for 'A'
	exiIn.V2G_Message.Body.PaymentDetailsReq.eMAID.characters[5] = 49;  // ASCII value for '1'	
	exiIn.V2G_Message.Body.PaymentDetailsReq.eMAID.characters[6] = 65;  // ASCII value for 'A'
	exiIn.V2G_Message.Body.PaymentDetailsReq.eMAID.characters[7] = 50;  // ASCII value for '2'
	exiIn.V2G_Message.Body.PaymentDetailsReq.eMAID.characters[8] = 66;  // ASCII value for 'B'
	exiIn.V2G_Message.Body.PaymentDetailsReq.eMAID.characters[9] = 51;  // ASCII value for '3'
	exiIn.V2G_Message.Body.PaymentDetailsReq.eMAID.characters[10] = 67; // ASCII value for 'C'
	exiIn.V2G_Message.Body.PaymentDetailsReq.eMAID.characters[11] = 52; // ASCII value for '4'
	exiIn.V2G_Message.Body.PaymentDetailsReq.eMAID.characters[12] = 68; // ASCII value for 'D'
	exiIn.V2G_Message.Body.PaymentDetailsReq.eMAID.characters[13] = 53; // ASCII value for '5'
	exiIn.V2G_Message.Body.PaymentDetailsReq.eMAID.characters[14] = 57; // ASCII value for '9'
	exiIn.V2G_Message.Body.PaymentDetailsReq.eMAID.charactersLen = 15; // Length of the eMAID


	exiIn.V2G_Message.Body.PaymentDetailsReq.ContractSignatureCertChain.Certificate.bytes[0] = 'M';
	exiIn.V2G_Message.Body.PaymentDetailsReq.ContractSignatureCertChain.Certificate.bytes[1] = 'I';
	exiIn.V2G_Message.Body.PaymentDetailsReq.ContractSignatureCertChain.Certificate.bytesLen = 2;

	exiIn.V2G_Message.Body.PaymentDetailsReq.ContractSignatureCertChain.SubCertificates_isUsed = 1u;

	exiIn.V2G_Message.Body.PaymentDetailsReq.ContractSignatureCertChain.SubCertificates.Certificate.array[0].bytes[0] = 'M';
	exiIn.V2G_Message.Body.PaymentDetailsReq.ContractSignatureCertChain.SubCertificates.Certificate.array[0].bytes[1] = 'I';	
	exiIn.V2G_Message.Body.PaymentDetailsReq.ContractSignatureCertChain.SubCertificates.Certificate.array[0].bytesLen = 2;

	exiIn.V2G_Message.Body.PaymentDetailsReq.ContractSignatureCertChain.SubCertificates.Certificate.array[1].bytes[0] = 'M';
	exiIn.V2G_Message.Body.PaymentDetailsReq.ContractSignatureCertChain.SubCertificates.Certificate.array[1].bytes[1] = 'I';
	exiIn.V2G_Message.Body.PaymentDetailsReq.ContractSignatureCertChain.SubCertificates.Certificate.array[1].bytes[2] = 'I';
	exiIn.V2G_Message.Body.PaymentDetailsReq.ContractSignatureCertChain.SubCertificates.Certificate.array[1].bytesLen = 3;
	exiIn.V2G_Message.Body.PaymentDetailsReq.ContractSignatureCertChain.SubCertificates.Certificate.arrayLen = 2;

	exiIn.V2G_Message.Body.PaymentDetailsReq.ContractSignatureCertChain.Id_isUsed = 1u;
	exiIn.V2G_Message.Body.PaymentDetailsReq.ContractSignatureCertChain.Id.charactersLen = 2;
	exiIn.V2G_Message.Body.PaymentDetailsReq.ContractSignatureCertChain.Id.characters[0] = 'M';
	exiIn.V2G_Message.Body.PaymentDetailsReq.ContractSignatureCertChain.Id.characters[1] = 'I';

	printf("EV side: call EVSE ServiceDetail \n");

	errn = request_response2(&exiIn, &exiOut, 0, 1428);
	if(errn == 0) {
			paymentDetailsRes = exiOut.V2G_Message.Body.PaymentDetailsRes;
			if (exiOut.V2G_Message.Body.PaymentDetailsRes.GenChallenge.bytesLen == 0 || exiOut.V2G_Message.Body.PaymentDetailsRes.GenChallenge.bytes == NULL) {
    // Gen_Challenge is empty or uninitialized
    printf("Gen_Challenge is empty or uninitialized\n");
} else {
    // Gen_Challenge is not empty
    printf("Gen_Challenge is not empty\n");
}

			printf("EV side: received response message from EVSE\n");
			/* show results of EVSEs answer message */
			printf("\tHeader SessionID=");
			printBinaryArray(exiOut.V2G_Message.Header.SessionID.bytes, exiOut.V2G_Message.Header.SessionID.bytesLen);
			printf("\t ResponseCode=%d\n",  paymentDetailsRes.ResponseCode);
			printf("\tEVSETimeStamp=%li\n",  (long int) paymentDetailsRes.EVSETimeStamp);
			printf("\tGenChallenge=%d\n",   paymentDetailsRes.GenChallenge.bytes[0]);
			printBinaryArray(paymentDetailsRes.GenChallenge.bytes, paymentDetailsRes.GenChallenge.bytesLen);

	} else {
		return errn;
	}

	/*******************************************
	 * Authorization *
	 *******************************************/
	init_iso2BodyType(&exiIn.V2G_Message.Body);
	exiIn.V2G_Message.Body.AuthorizationReq_isUsed = 1u;

	init_iso2AuthorizationReqType(&exiIn.V2G_Message.Body.AuthorizationReq);

	copyBytes(paymentDetailsRes.GenChallenge.bytes, paymentDetailsRes.GenChallenge.bytesLen, exiIn.V2G_Message.Body.AuthorizationReq.GenChallenge.bytes);
	exiIn.V2G_Message.Body.AuthorizationReq.GenChallenge.bytesLen = paymentDetailsRes.GenChallenge.bytesLen;
	exiIn.V2G_Message.Body.AuthorizationReq.GenChallenge_isUsed = 1u; /* no challenge needed here*/
	exiIn.V2G_Message.Body.AuthorizationReq.Id_isUsed = 1u; /* no signature needed here */
	exiIn.V2G_Message.Body.AuthorizationReq.Id.charactersLen = 5;
	exiIn.V2G_Message.Body.AuthorizationReq.Id.characters[0] = 'I';
	exiIn.V2G_Message.Body.AuthorizationReq.Id.characters[1] = 'D';
	exiIn.V2G_Message.Body.AuthorizationReq.Id.characters[2] = '0';
	exiIn.V2G_Message.Body.AuthorizationReq.Id.characters[3] = '0';
	exiIn.V2G_Message.Body.AuthorizationReq.Id.characters[4] = '1';

	printf("EV side: call EVSE Authorization \n");
	printf("ID: %c%c%c%c%c\n",
       exiIn.V2G_Message.Body.AuthorizationReq.Id.characters[0],
       exiIn.V2G_Message.Body.AuthorizationReq.Id.characters[1],
       exiIn.V2G_Message.Body.AuthorizationReq.Id.characters[2],
       exiIn.V2G_Message.Body.AuthorizationReq.Id.characters[3],
       exiIn.V2G_Message.Body.AuthorizationReq.Id.characters[4]);

if (exiIn.V2G_Message.Body.AuthorizationReq.GenChallenge.bytesLen == 0 || exiIn.V2G_Message.Body.AuthorizationReq.GenChallenge.bytes == NULL) {
    // Gen_Challenge is empty or uninitialized
    printf("Gen_Challenge is empty or uninitialized\n");
} else {
    // Gen_Challenge is not empty
    printf("Gen_Challenge is not empty\n");
}

printf("Gen_Challenge: ");
for (int i = 0; i < exiIn.V2G_Message.Body.AuthorizationReq.GenChallenge.bytesLen; i++) {
    printf( "%20X ", exiIn.V2G_Message.Body.AuthorizationReq.GenChallenge.bytes[i]);
}
printf("\n");
	errn = request_response2(&exiIn, &exiOut, 0, 319);
	if(errn == 0) {
		/* check, if this is the right response message */
		if(exiOut.V2G_Message.Body.AuthorizationRes_isUsed) {

			/* show results of EVSEs answer message */
			printf("EV side: received response message from EVSE\n");
			printf("\tHeader SessionID=");
			printBinaryArray(exiOut.V2G_Message.Header.SessionID.bytes, exiOut.V2G_Message.Header.SessionID.bytesLen);
			printf("\t ResponseCode=%d\n",  exiOut.V2G_Message.Body.AuthorizationRes.ResponseCode);

			if(exiOut.V2G_Message.Body.AuthorizationRes.EVSEProcessing == iso2EVSEProcessingType_Finished) {
				printf("\t EVSEProcessing=Finished\n");
			}
		} else {
			errn = ERROR_UNEXPECTED_AUTHORIZATION_RESP_MESSAGE;
			return errn;
		}
	} else {
		return errn;
	}


	/*******************************************
	 * chargeParameterDiscovery *
	 *******************************************/
	printf("EV side: call EVSE chargeParameterDiscovery");

	 errn = chargingpar(&exiIn, &exiOut, 0);

	if(errn == 0) {

			/* show results of EVSEs answer message */
			printf("EV side: received response message from EVSE\n");
		} else {
			errn = ERROR_UNEXPECTED_CHARGE_PARAMETER_DISCOVERY_RESP_MESSAGE;
			return errn;
		}

	/*****************************
	 * cableCheck *
	 *****************************/
	init_iso2BodyType(&exiIn.V2G_Message.Body);
	exiIn.V2G_Message.Body.CableCheckReq_isUsed = 1u;

	/*init_v2gCableCheckReqType(&exiIn.V2G_Message.Body.CableCheckReq);*/

	printf("EV side: call EVSE cableCheck \n");

	errn = cablecheck(&exiIn, &exiOut, 0);

	if(errn == 0) {

			/* show results of EVSEs answer message */
			printf("EV side: received response message from EVSE\n");

		} else {
			errn = ERROR_UNEXPECTED_CABLE_CHECK_RESP_MESSAGE;
			return errn;
		}

	/*****************************
	 * preCharge *
	 *****************************/
	errn = precharge(&exiIn, &exiOut, 0);

	if(errn == 0) {
		/* check, if this is the right response message */
		printf("safe");
	} else {
		return errn;
	}

	/*********************************
	 * PowerDelivery *
	 *********************************/

	init_iso2BodyType(&exiIn.V2G_Message.Body);
	exiIn.V2G_Message.Body.PowerDeliveryReq_isUsed = 1u;
	printf("EV side: call EVSE powerDelivery \n");

	errn = power(&exiIn, &exiOut, 0);

	if(errn == 0) {
			printf("EV side: received response message from EVSE\n");
	} else {
		return errn;
	}

errn = startcharge(&exiIn, &exiOut, 0, Charginglevel);

 if(errn == 0) {
                        printf("EV side: received response message from EVSE\n");
        } else {
                return errn;
        }

	/***********************************
	 * SessionStop *
	 ***********************************/


	init_iso2BodyType(&exiIn.V2G_Message.Body);
	exiIn.V2G_Message.Body.SessionStopReq_isUsed = 1u;

	init_iso2SessionStopReqType(&exiIn.V2G_Message.Body.SessionStopReq);
	exiIn.V2G_Message.Body.SessionStopReq.ChargingSession = iso2chargingSessionType_Terminate;

	printf("EV side: call EVSE stopSession \n");

	errn = request_response2(&exiIn, &exiOut, 0 ,22);

return errn;
}

#endif /* DEPLOY_ISO2_CODEC == SUPPORT_YES */
int  ChargingLevel;
int main_example(int argc, char *argv[]) {
if (argc < 2) {
    return 1;
}
    ChargingLevel = atoi(argv[1]);
	printf("Charging Level:	%d\n", ChargingLevel);
    int errn = 0;
	printf("+++ Start application handshake protocol example +++\n\n");
	errn = appHandshake();
	printf("+++ Terminate application handshake protocol example with errn = %d +++\n\n", errn);

	if(errn != 0) {
		printf("\n\nHandshake error %d!\n", errn);
		return errn;
	}
#if DEPLOY_ISO2_CODEC == SUPPORT_YES
    printf("+++ Start V2G client / service example for charging (ISO2) +++\n\n");
    // Call the charging2 function
   errn = charging2(ChargingLevel);

    if (errn == 0) {
        printf("+++Terminate V2G Client / Service example for charging successfully.+++\n\n");
    } else {
        printf("\n\ncharging error %d!\n", errn);
        return errn;
    }
#endif /* DEPLOY_ISO2_CODEC == SUPPORT_YES */
    return errn;
}
int main(int argc, char *argv[]) {
    char message[] = {0x01, 0xFE, 0x90, 0x00, 0x00, 0x00, 0x00, 0x02, 0x10, 0x00};
    char response_buffer[BUFFER_SIZE];
    // Create a UDP socket
    if ((client_socket = socket(AF_INET6, SOCK_DGRAM, 0)) == -1) {
        error("Error creating socket");
    }

    // Check if the charging level argument was provided
    if (argc > 1) {
        // Convert the charging level argument to an integer
      int  charging_level = atoi(argv[1]);
	printf("charing level: %d\n", charging_level);
    }

    // Initialize server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(SERVER_PORT);

    int conversion_result = inet_pton(AF_INET6, SERVER_IP, &server_addr.sin6_addr);
    if (conversion_result != 1) {
        perror("Error converting IP address");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Send data
    if (sendto(client_socket, message, sizeof(message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        error("Error sending data");
    }

    printf("Message sent to the server\n");

    // Receive response from the server
    struct sockaddr_in6 client_addr;
    socklen_t addr_len = sizeof(client_addr);
    ssize_t received_bytes;

    received_bytes = recvfrom(client_socket, response_buffer, sizeof(response_buffer), 0, (struct sockaddr*)&client_addr, &addr_len);

    if (received_bytes == -1) {
        perror("Error receiving data");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Received response from the server: ");
    for (ssize_t i = 0; i < received_bytes; ++i) {
        printf("%02X ", response_buffer[i]);
    }
    printf("\n");
    /* disable buffering of output, especially when piped or redirected */
    setvbuf(stdout, NULL, _IONBF, 0);

    #if CODE_VERSION == CODE_VERSION_EXI
        /* EXI codec only */
       return main_databinder(argc, argv);
   #elif CODE_VERSION == CODE_VERSION_SAMPLE
        /* V2G client / service example  */
        return main_example(argc, argv);
    #endif
close(client_socket);

    return 0;
}


