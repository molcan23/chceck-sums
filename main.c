#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "argtable3.h"

#define CRC_START_32          0xFFFFFFFF
#define CRC_START_DNP         0x0000
#define CRC_START_KERMIT      0x0000
#define CRC_START_SICK        0x0000
#define CRC_POLY_SICK         0x8005
#define CRC_POLY_KERMIT       0x8408
#define CRC_POLY_DNP          0xA6BC
#define CRC_POLY_32           0xEDB88320ul
#define velkost 1024


 typedef unsigned char byte;
 
static uint16_t        crc_tab[256];
static bool        crc_tab_init        = false;

static void init_crc_tab( void ) {

    uint16_t i;
    uint16_t j;
    uint16_t crc;
    uint16_t c;

    for (i=0; i<256; i++) {

        crc = 0;
        c   = i;

        for (j=0; j<8; j++) {

            if ( (crc ^ c) & 0x0001 ) crc = ( crc >> 1 ) ^ CRC_POLY_KERMIT;
            else                      crc =   crc >> 1;

            c = c >> 1;
        }

        crc_tab[i] = crc;
    }

    crc_tab_init = true;

} 

static bool crc_tabdnp_init = false;
static uint16_t crc_tabdnp[256];


static void init_crcdnp_tab( void ) {

    int i;
    int j;
    uint16_t crc;
    uint16_t c;

    for (i=0; i<256; i++) {

        crc = 0;
        c   = (uint16_t) i;

        for (j=0; j<8; j++) {

            if ( (crc ^ c) & 0x0001 ) crc = ( crc >> 1 ) ^ CRC_POLY_DNP;
            else                      crc =   crc >> 1;

            c = c >> 1;
        }

        crc_tabdnp[i] = crc;
    }

    crc_tabdnp_init = true;

} 

static bool            crc_tab32_init = false;
static uint32_t        crc_tab32[256];

static void init_crc32_tab( void ) {

    int i, j;
    unsigned long crc;

    for (i=0; i<256; i++) {

        crc = (unsigned long) i;

        for (j=0; j<8; j++) {

            if ( crc & 0x00000001L ) crc = ( crc >> 1 ) ^ CRC_POLY_32;
            else                     crc =   crc >> 1;
        }

        crc_tab32[i] = crc;
    }

    crc_tab32_init = true;

} 

//inspired by http://www.hackersdelight.org/hdcodetxt/crc.c.txt
uint32_t crc32( const unsigned char *input_str, size_t num_bytes ) {

    uint32_t crc;
    uint32_t tmp;
    uint32_t long_c;
    const unsigned char *ptr;
    size_t a;

    if ( ! crc_tab32_init ) init_crc32_tab();

    crc = CRC_START_32;
    ptr = input_str;

    if ( ptr != NULL ) for (a=0; a<num_bytes; a++) {

        long_c = 0x000000FFL & (uint32_t) *ptr;
        tmp    =  crc       ^ long_c;
        crc    = (crc >> 8) ^ crc_tab32[ tmp & 0xff ];

        ptr++;
    }

    crc ^= 0xffffffffL;

    return crc & 0xffffffffL;

} 

//**1**
//crc DNP
uint16_t crc_dnp( const unsigned char *input_str, size_t num_bytes ) {

    uint16_t crc;
    uint16_t tmp;
    uint16_t short_c;
    uint16_t low_byte;
    uint16_t high_byte;
    const unsigned char *ptr;
    size_t a;

    if ( ! crc_tabdnp_init ) init_crcdnp_tab();

    crc = CRC_START_DNP;
    ptr = input_str;

    if ( ptr != NULL ) for (a=0; a<num_bytes; a++) {

        short_c = 0x00ff & (uint16_t) *ptr;
        tmp     =  crc       ^ short_c;
        crc     = (crc >> 8) ^ crc_tabdnp[ tmp & 0xff ];

        ptr++;
    }

    crc       = ~crc;
    low_byte  = (crc & 0xff00) >> 8;
    high_byte = (crc & 0x00ff) << 8;
    crc       = low_byte | high_byte;

    return crc;

}
 
//crc KERMIT
//**1**
 uint16_t crc_kermit( const unsigned char *input_str, size_t num_bytes ) {

    uint16_t crc;
    uint16_t tmp;
    uint16_t short_c;
    uint16_t low_byte;
    uint16_t high_byte;
    const unsigned char *ptr;
    size_t a;

    if ( ! crc_tab_init ) init_crc_tab();

    crc = CRC_START_KERMIT;
    ptr = input_str;

    if ( ptr != NULL ) for (a=0; a<num_bytes; a++) {

        short_c = 0x00ff & (uint16_t) *ptr;
        tmp     =  crc       ^ short_c;
        crc     = (crc >> 8) ^ crc_tab[ tmp & 0xff ];

        ptr++;
    }

    low_byte  = (crc & 0xff00) >> 8;
    high_byte = (crc & 0x00ff) << 8;
    crc       = low_byte | high_byte;

    return crc;

}
 
//crc SICK
uint16_t crc_sick( const unsigned char *input_str, size_t num_bytes ) {

    uint16_t crc;
    uint16_t low_byte;
    uint16_t high_byte;
    uint16_t short_c;
    uint16_t short_p;
    const unsigned char *ptr;
    size_t a;

    crc     = CRC_START_SICK;
    ptr     = input_str;
    short_p = 0;

    if ( ptr != NULL ) for (a=0; a<num_bytes; a++) {

        short_c = 0x00ff & (uint16_t) *ptr;

        if ( crc & 0x8000 ) crc = ( crc << 1 ) ^ CRC_POLY_SICK;
        else                crc =   crc << 1;

        crc    ^= ( short_c | short_p );
        short_p = short_c << 8;

        ptr++;
    }

    low_byte  = (crc & 0xff00) >> 8;
    high_byte = (crc & 0x00ff) << 8;
    crc       = low_byte | high_byte;

    return crc;

} 

// Dallas-crc16 
uint16_t crc16( uint8_t const *data, int len){
     uint16_t crc=0;
     
     for (int i=0; i<len;i++)
     {
           uint8_t inbyte = data[i];
           for (uint8_t j=0;j<8;j++)
           {
                 uint8_t mix = (crc^ inbyte) & 0x01;
                 crc >>= 1;
                 if (mix)
                       crc ^= 0xA001;
                 
                 inbyte >>= 1;
           }
     }
     return crc;
}

//Maxim-crc8 
uint8_t crc8( uint8_t const *addr, int len){
     uint8_t crc=0;

     for (int i=0; i<len;i++)
     {
           uint8_t inbyte = addr[i];
           for (uint8_t j=0;j<8;j++)
           {
                 uint8_t mix = (crc ^ inbyte) & 0x01;
                 crc >>= 1;
                 if (mix)
                       crc ^= 0x8C;
                 
                 inbyte >>= 1;
           }
     }
     return crc;
}
  
const uint32_t MOD_ADLER = 65521;

//https://en.wikipedia.org/wiki/Adler-32
uint32_t adler32(unsigned char *data, size_t len) {
    uint32_t a = 1, b = 0;
    size_t index;
    
    for (index = 0; index < len; ++index)
    {
        a = (a + data[index]) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }
    
    return (b << 16) | a;
}

//https://stackoverflow.com/questions/13491700/8-bit-fletcher-checksum-of-16-byte-data

#define MAXPART 5803   

uint8_t fletcher8(unsigned char *data, size_t len)
{
    unsigned f8 = 1; 
    unsigned long sum1, sum2;
    size_t part;

    sum1 = f8 & 0xf;
    sum2 = (f8 >> 4) & 0xf;
    while (len) {
        part = len > MAXPART ? MAXPART : len;
        len -= part;
        do {
            sum2 += sum1 += *data++;
        } while (--part);
        sum1 %= 15;
        sum2 %= 15;
    }
    return (sum2 << 4) + sum1;
}

//https://en.wikipedia.org/wiki/Fletcher%27s_checksum
uint16_t fletcher16( uint8_t const *data, size_t count ){

   uint16_t sum1 = 0;
   uint16_t sum2 = 0;
   int index;

   for( index = 0; index < count; ++index ){
      sum1 = (sum1 + data[index]) % 255;
      sum2 = (sum2 + sum1) % 255;
   }

   return (sum2 << 8) | sum1;
}

//https://en.wikipedia.org/wiki/Fletcher%27s_checksum
uint32_t fletcher32(const uint16_t *data, size_t len)
{
        uint32_t c0, c1;
        unsigned int i;

        for (c0 = c1 = 0; len >= 360; len -= 360) {
                for (i = 0; i < 360; ++i) {
                        c0 = c0 + *data++;
                        c1 = c1 + c0;
                }
                c0 = c0 % 65535;
                c1 = c1 % 65535;
        }
        for (i = 0; i < len; ++i) {
                c0 = c0 + *data++;
                c1 = c1 + c0;
        }
        c0 = c0 % 65535;
        c1 = c1 % 65535;
        return (c1 << 16 | c0);
}

uint8_t bsdChecksum8(uint8_t const *data, int size8) {
    uint8_t sum = 0;

    while (size8-- > 0) {
        sum = (sum >> 1) | (sum << 7);
        sum += *(data++);
    }

    return sum;
}

uint16_t bsdChecksum16(uint16_t const *data, int size16) {
    uint16_t sum = 0;

    while (size16-- > 0) {
        sum = (sum >> 1) | (sum << 15);
        sum += *(data++);
    }

    return sum;
}

uint8_t sum8(uint8_t const *data, int size8) {
    uint8_t sum = 0;

    while (size8-- > 0) {
        sum += *(data++);
    }

    return sum;
}

uint16_t parity16(uint16_t const *data, int size16) {
    uint16_t sum = 0;

    while (size16-- > 0) {
        sum = sum | *(data++);
    }
    return sum;
}

uint8_t parity8(uint8_t const *data, int size8) {
    uint8_t sum = 0;

    while (size8-- > 0) {
        sum = sum | *(data++);
    }
    return sum;
}

void readFileIntoMemory(char const *const filename, void **buffer, size_t *size8) {
    FILE *fh = fopen(filename, "rb");
    assert (fh != NULL);

    fseek(fh, 0L, SEEK_END);
    *size8 = (size_t) ftell(fh);
    rewind(fh);

    *buffer = malloc(*size8);
    assert(*buffer != NULL);

    size_t read = fread(*buffer, *size8, 1, fh);
    assert(read == 1);

    fclose(fh);
}

char testResult(int val, int const *const knownVal) {
    if (knownVal == NULL) return ' ';
    return (*knownVal == val) ? '*' : '-';
}

unsigned int number_length(uint32_t x) {
    unsigned int len = 0;
    while (x > 0) {
        x /= 16;
        len++;
    }
    return len;
}

void calculateAllChecksums(char const *const filename, int const *const knownVal) {

    void *buffer;
    size_t size8;
    readFileIntoMemory(filename, &buffer, &size8);

    uint8_t  s8;
    uint16_t s16;
    uint32_t s32;
    const char *padding = "                                        ";
    unsigned int len = 0;
    const unsigned int max_len = 11;

    s8 = bsdChecksum8(buffer, size8);
    len = number_length((uint32_t)s8);

    printf("%c bsdChecksum8  = %*.*s0x%x (%d)\n", testResult(s8, knownVal), max_len - len, max_len - len, padding, s8, s8);

    s16 = bsdChecksum16(buffer, size8 / 2);
    len = number_length((uint32_t)s16);
    printf("%c bsdChecksum16 = %*.*s0x%x (%d)\n", testResult(s16, knownVal), max_len - len, max_len - len, padding, s16, s16);

    s8 = sum8(buffer, size8);
    len = number_length((uint32_t)s8);
    printf("%c         sum8  = %*.*s0x%x (%d)\n", testResult(s8, knownVal), max_len - len, max_len - len, padding, s8, s8);

    s8 = parity8(buffer, size8);
    len = number_length((uint32_t)s8);
    printf("%c      parity8  = %*.*s0x%x (%d)\n", testResult(s8, knownVal), max_len - len, max_len - len, padding, s8, s8);

    s16 = parity16(buffer, size8 / 2);
    len = number_length((uint32_t)s16);
    printf("%c     parity16  = %*.*s0x%x (%d)\n", testResult(s16, knownVal), max_len - len, max_len - len, padding, s16, s16);

    s8 = fletcher8(buffer, size8);
    len = number_length((uint32_t)s8);
    printf("%c    fletcher8  = %*.*s0x%x (%d)\n", testResult(s8, knownVal), max_len - len, max_len - len, padding, s8, s8);

    s16 = fletcher16(buffer, size8);
    len = number_length((uint32_t)s16); 
    printf("%c   fletcher16  = %*.*s0x%x (%d)\n", testResult(s16, knownVal), max_len - len, max_len - len, padding, s16, s16);

    s32 = fletcher32(buffer, size8 / 2);
    len = number_length((uint32_t)s32); 
    printf("%c   fletcher32  = %*.*s0x%x (%d)\n", testResult(s32, knownVal), max_len - len, max_len - len, padding, s32, s32);

    s32 = adler32(buffer, size8);
    len = number_length((uint32_t)s32);
    printf("%c      adler32  = %*.*s0x%x (%d)\n", testResult(s32, knownVal), max_len - len, max_len - len, padding, s32, s32);

    s8 = crc8(buffer, size8);
    len = number_length((uint32_t)s8);
    printf("%c   Maxim-crc8  = %*.*s0x%x (%d)\n", testResult(s8, knownVal), max_len - len, max_len - len, padding, s8, s8);

    s16 = crc16(buffer, size8);
    len = number_length((uint32_t)s16);
    printf("%c Dallas-crc16  = %*.*s0x%x (%d)\n", testResult(s16, knownVal), max_len - len, max_len - len, padding, s16, s16);

    s16 = crc32(buffer, size8);
    len = number_length((uint32_t)s16);
    printf("%c        crc32  = %*.*s0x%x (%d)\n", testResult(s16, knownVal), max_len - len, max_len - len, padding, s16, s16);

    s16 = crc_sick(buffer, size8); 
    len = number_length((uint32_t)s16);
    printf("%c     crc SICK  = %*.*s0x%x (%d)\n", testResult(s16, knownVal), max_len - len, max_len - len, padding, s16, s16);

    s16 = crc_kermit(buffer, size8);
    len = number_length((uint32_t)s16);
    printf("%c   crc KERMIT  = %*.*s0x%x (%d)\n", testResult(s16, knownVal), max_len - len, max_len - len, padding, s16, s16);

    s16 = crc_dnp(buffer, size8);
    len = number_length((uint32_t)s16);
    printf("%c      crc DNP  = %*.*s0x%x (%d)\n", testResult(s16, knownVal), max_len - len, max_len - len, padding, s16, s16);


    free(buffer);
}

int tests() {

    int errors = 0;
    uint8_t  s8;
    uint16_t s16;
    uint32_t s32;
    size_t size8;
    FILE *fp;
    char line[velkost];
    char buff[25];
    char kod[20];
    char Sucet[25];
    int kontrolnySucet;
    size_t nread;

    FILE *file;

    file = fopen("tests.txt", "r");
    if (file) {

        while ((fgets(line, sizeof line, file)) != NULL){
        if(line[0] != '#'){                            
            if(line[0] == '['){                        
                strncpy(kod, line, sizeof(kod));        
                int len = strlen(kod);
                if (len > 0 && kod[len-1] == '\n') kod[len-1] = '\0';
            }else{                                
                char *token;
                token = strtok(line, " ");                
                strncpy(buff, token, sizeof(buff));
                
                token = strtok(NULL, " ");
                strncpy(Sucet, token, sizeof(Sucet));
                
                int len = strlen(Sucet);
                if (len > 0 && Sucet[len-1] == '\n') Sucet[len-1] = '\0';

                kontrolnySucet = (int)strtol(Sucet, NULL, 0);

                size8 = strlen(buff);                    

                if (strcmp(kod, "[bsd8]") == 0){
                    s8 = bsdChecksum8(buff, size8);                        
                    if(s8 != kontrolnySucet) {
                        printf("Error in bsd8 at %s. Result was 0x%x, expected 0x%x.\n",buff,s8,kontrolnySucet);
                        errors++;
                    }
                }    

                if (strcmp(kod, "[bsd16]") == 0){
                    s16 = bsdChecksum16(buff, size8);
                    if(s16 != kontrolnySucet) {
                        printf("Error in bsd16 at %s. Result was 0x%x, expected 0x%x.\n",buff,s16,kontrolnySucet);
                        errors++;
                    }
                }

                if (strcmp(kod, "[sum8]") == 0){
                    s8 = sum8(buff, size8);
                    if(s8 != kontrolnySucet) {
                        printf("Error in sum8 at %s. Result was 0x%x, expected 0x%x.\n",buff,s8,kontrolnySucet);
                        errors++;
                    }
                }

                if (strcmp(kod, "[parity8]") == 0){
                    s8 = parity8(buff, size8);
                    if(s8 != kontrolnySucet) {
                        printf("Error in parity8 at %s. Result was 0x%x, expected 0x%x.\n",buff,s8,kontrolnySucet);
                        errors++;
                    }
                }

                if (strcmp(kod, "[parity16]") == 0){
                    s16 = parity16(buff, size8 / 2);
                    if(s16 != kontrolnySucet) {
                        printf("Error in parity16 at %s. Result was 0x%x, expected 0x%x.\n",buff,s16,kontrolnySucet);
                        errors++;
                    }
                }

                if (strcmp(kod, "[flecher8]") == 0){
                    s8 = fletcher8(buff, size8);
                    if(s8 != kontrolnySucet) {
                        printf("Error in fletcher8 at %s. Result was 0x%x, expected 0x%x.\n",buff,s8,kontrolnySucet);
                        errors++;
                    }
                }

                if (strcmp(kod, "[fletcher16]") == 0){
                    s16 = fletcher16(buff, size8);
                    if(s16 != kontrolnySucet) {
                        printf("Error in fletcher16 at %s. Result was 0x%x, expected 0x%x.\n",buff,s16,kontrolnySucet);
                        errors++;
                    }
                }

                if (strcmp(kod, "[fletcher32]") == 0){
                    s32 = fletcher32(buff, size8);
                    if(s32 != kontrolnySucet) {
                        printf("Error in fletcher32 at %s. Result was 0x%x, expected 0x%x.\n",buff,s32,kontrolnySucet);
                        errors++;
                    }
                }

                if (strcmp(kod, "[adler32]") == 0){
                    s32 = adler32(buff, size8);
                    if(s32 != kontrolnySucet) {
                        printf("Error in adler32 at %s. Result was 0x%x, expected 0x%x.\n",buff,s32,kontrolnySucet);
                        errors++;
                    }
                }

                if (strcmp(kod, "[crc8]") == 0){
                    s8 = crc8(buff, size8);
                    if(s8 != kontrolnySucet) {
                        printf("Error in Maxim-crc8 at %s. Result was 0x%x, expected 0x%x.\n",buff,s8,kontrolnySucet);
                        errors++;
                    }
                }

                if (strcmp(kod, "[crc16]") == 0){
                    s16 = crc16(buff, size8);
                    if(s16 != kontrolnySucet) {
                        printf("Error in Dallas-crc16 at %s. Result was 0x%x, expected 0x%x.\n",buff,s16,kontrolnySucet);
                        errors++;
                    }
                }    

                if (strcmp(kod, "[crc32]") == 0){
                    s32 = crc32(buff, size8);
                    if(s32 != kontrolnySucet) {
                        printf("Error in crc32 at %s. Result was 0x%x, expected 0x%x.\n",buff,s32,kontrolnySucet);
                        errors++;
                    }
                }


                if (strcmp(kod, "[crc_sick]") == 0){
                    s16 = crc_sick(buff, size8); 
                    if(s16 != kontrolnySucet) {
                        printf("Error in crc SICK at %s. Result was 0x%x, expected 0x%x.\n",buff,s16,kontrolnySucet);
                        errors++;
                    }

                }

                if (strcmp(kod, "[crc_kermit]") == 0){
                    s16 = crc_kermit(buff, size8); 
                    if(s16 != kontrolnySucet) {
                        printf("Error in crc KERMIT at %s. Result was 0x%x, expected 0x%x.\n",buff,s16,kontrolnySucet);
                        errors++;
                    }
                }

                if (strcmp(kod, "[crc_dnp]") == 0){            
                    s16 = crc_dnp(buff, size8); 
                    if(s16 != kontrolnySucet) {
                        printf("Error in crc DNP at %s. Result was 0x%x, expected 0x%x.\n",buff,s16,kontrolnySucet);
                        errors++;
                    }
                }    

            }        
        
        }

        }
        fclose(file);

    }

    return errors;
}

void find_method(int kontrolnySucet, char const *const filename) {

    void *buff;
    size_t size8;
    readFileIntoMemory(filename, &buff, &size8);
    uint8_t  s8;
    uint16_t s16;
    uint32_t s32;
    int found = 0;

    s8 = bsdChecksum8(buff, size8);                        
    if(s8 == kontrolnySucet) {
        printf("Maybe bsd8 was used.\n");
        found++;
    }

    s16 = bsdChecksum16(buff, size8);
    if(s16 == kontrolnySucet) {
        printf("Maybe bsd16 was used.\n");
        found++;
    }

    s8 = sum8(buff, size8);
    if(s8 == kontrolnySucet) {
        printf("Maybe sum8 was used.\n");
        found++;
    }

    s8 = parity8(buff, size8);
    if(s8 == kontrolnySucet) {
        printf("Maybe parity8 was used.\n");
        found++;
    }

    s16 = parity16(buff, size8 / 2);
    if(s16 == kontrolnySucet) {
        printf("Maybe parity16 was used.\n");
        found++;
    }

    s8 = fletcher8(buff, size8);
    if(s8 == kontrolnySucet) {
        printf("Maybe fletcher8 was used.\n");
        found++;
    }

    s16 = fletcher16(buff, size8);
    if(s16 == kontrolnySucet) {
        printf("Maybe fletcher16 was used.\n");
        found++;
    }

    s32 = fletcher32(buff, size8);
    if(s32 == kontrolnySucet) {
        printf("Maybe fletcher32 was used.\n");
        found++;
    }

    s32 = adler32(buff, size8);
    if(s32 == kontrolnySucet) {
        printf("Maybe adler32 was used.\n");
        found++;
    }

    s8 = crc8(buff, size8);
    if(s8 == kontrolnySucet) {
        printf("Maybe Maxim-crc8 was used.\n");
        found++;
    }

    s16 = crc16(buff, size8);
    if(s16 == kontrolnySucet) {
        printf("Maybe Dallas-crc16 was used.\n");
        found++;
    }

    s32 = crc32(buff, size8);
    if(s32 == kontrolnySucet) {
        printf("Maybe crc32 was used.\n");
        found++;
    }

    s16 = crc_sick(buff, size8); 
    if(s16 == kontrolnySucet) {
        printf("Maybe crc SICK was used.\n");
        found++;
    }

    s16 = crc_kermit(buff, size8); 
    if(s16 == kontrolnySucet) {
        printf("Maybe crc KERMIT was used.\n");
        found++;
    }

    s16 = crc_dnp(buff, size8); 
    if(s16 == kontrolnySucet) {
        printf("Maybe crc DNP was used.\n");
        found++;
    }


    if(found==0)printf("No matching method.\n");
}

const char progname[] = "chsum";

struct arg_lit *help, *test, *find, *all;
struct arg_int *val;
struct arg_file *file;
struct arg_end *end;

int main(int argc, char *argv[]) {
    void *argtable[] = {
            help = arg_litn(NULL, "help", 0, 1, "display this help and exit"),
            all = arg_litn("a", "all", 0, 1, "calculate all types of checksums implemented for input file"),
            test = arg_litn("t", "test", 0, 1, "testing"),
            find = arg_litn("f", "find", 0, 1, "find type of crc"),
            val = arg_intn("v", NULL, "<n>", 0, 1, "known value of checksum"),
            file = arg_filen(NULL, NULL, "<file>", 1, 1, "input file"),
            end = arg_end(20),
    };

    int nerrors = arg_parse(argc, argv, argtable);

    int exitcode = EXIT_SUCCESS;

    if (help->count > 0) {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        printf("Calculate different checksums of given file.\n\n");
        arg_print_glossary(stdout, argtable, "  %-15s %s\n");
        goto exit;
    }

    if(test->count > 0) {
        printf("Testing.\n");
        int x = tests();
        if(x==0){
            printf("Test successful\n");
        } else {
            printf("Test ended with %d errors\n", x);
            exitcode = EXIT_FAILURE;
        }
        goto exit;
    }

    if (nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        exitcode = EXIT_FAILURE;
        goto exit;
    }

    if (all->count > 0){
        if(file->count == 0){                        
            printf("Input file is missing.\n");
            exitcode = EXIT_FAILURE;
        } else {
            calculateAllChecksums(*file->filename, (val->count > 0) ? val->ival : NULL);
        }
        goto exit;
    }

    if (find->count > 0){
        if(file->count == 0){                        
            printf("Input file is missing.\n");
            exitcode = EXIT_FAILURE;
            goto exit;
        }
        if (val-> count == 0) {
            printf("Known value of checksum is missing (use -v).\n");
            exitcode = EXIT_FAILURE;
            goto exit;
        }
        
        find_method(*val->ival, *file->filename);
        goto exit;
    }
    
    printf("Need some option (-a, -f, -t). See --help.");
    exitcode = EXIT_FAILURE;
    
    exit:
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return exitcode;
}

// **1** inspired or copied from https://github.com/lammertb/libcrc
