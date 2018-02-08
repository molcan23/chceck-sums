#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include <stdbool.h>
#include "argtable3.h"

#define CRC_START_32	 	0xFFFFFFFF
#define CRC_START_DNP 		0x0000
#define CRC_START_KERMIT 	0x0000
#define CRC_START_SICK 		0x0000
#define CRC_POLY_SICK 		0x8005
#define	CRC_POLY_KERMIT		0x8408
#define	CRC_POLY_DNP		0xA6BC
#define	CRC_POLY_32		0xEDB88320ul


 typedef unsigned char byte;
 
static uint16_t		crc_tab[256];
static bool		crc_tab_init		= false;

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

static bool             crc_tabdnp_init         = false;
static uint16_t         crc_tabdnp[256];


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

static bool             crc_tab32_init          = false;
static uint32_t		crc_tab32[256];

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
uint32_t crc_32( const unsigned char *input_str, size_t num_bytes ) {

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
 
//KERMIT
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
 
//SICK

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
 
 bool damm(int const *input, int length) {
    static const unsigned char table[10][10] = {
        {0, 3, 1, 7, 5, 9, 8, 6, 4, 2},
        {7, 0, 9, 2, 1, 5, 4, 8, 6, 3},
        {4, 2, 0, 6, 8, 7, 1, 3, 5, 9},
        {1, 7, 5, 0, 9, 8, 3, 4, 2, 6},
        {6, 1, 2, 3, 0, 4, 5, 9, 7, 8},
        {3, 6, 7, 4, 2, 0, 9, 5, 8, 1},
        {5, 8, 6, 9, 7, 2, 0, 1, 3, 4},
        {8, 9, 4, 5, 3, 6, 2, 0, 1, 7},
        {9, 4, 3, 8, 6, 1, 7, 2, 0, 5},
        {2, 5, 8, 1, 4, 3, 6, 7, 9, 0},
    };
 
    int interim = 0;
    for (int i = 0; i < length; i++) {
        interim = table[interim][input[i]];
    }
    return interim == 0;
}

// Dallas 
uint16_t crc16( uint8_t *data, int len){
     uint16_t crc=0;
     
     for (uint8_t i=0; i<len;i++)
     {
           uint8_t inbyte = data[i];
           for (uint8_t j=0;j<8;j++)
           {
                 uint8_t mix = (crc^ inbyte) & 0x01;
                 crc = crc >> 1;
                 if (mix)
                       crc = crc ^ 0xA001;
                 
                 inbyte = inbyte >> 1;
           }
     }
     return crc;
}


//Maxim 

uint8_t crc8( uint8_t *addr, int len)
{
     uint8_t crc=0;
     
     for (uint8_t i=0; i<len;i++)
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


//SYSV          
             

uint16_t sysv_sum_file (uint16_t const *data, int size16){

  uint16_t sum = 0;
  
    int r;
    unsigned int s = 0;
  
  
  while (size16-- > 0) {
     r = (s & 0xffff) + ((s & 0xffffffff) >> 16);
     sum = (r & 0xffff) + (r >> 16); 
  }

  return sum;
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
uint16_t Fletcher16( uint8_t const *data, size_t count ){

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

void createDemoFile(char const *const filename, int const *const records) {
    int nrec = 4;
    if (records != NULL) {
        nrec = *records;
        if (nrec < 0) nrec = 0;
    }

    FILE *fh = fopen(filename, "wb");
    assert(fh != NULL);

    uint32_t x = 4;
    while (nrec--)
        fwrite(&x, sizeof(x), 1, fh);

    fclose(fh);
}

void readFileIntoMemory(char const *const filename, void **buffer, size_t *size8) {
    FILE *fh = fopen(filename, "rb");
    assert (fh != NULL);

    // get file size in bytes
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

void calculateAllChecksums(char const *const filename, int const *const knownVal) {
    void *buffer;
    size_t size8;
    readFileIntoMemory(filename, &buffer, &size8);

    uint8_t s8;
    uint16_t s16;

    s8 = bsdChecksum8(buffer, size8);
    printf("%c bsdChecksum8  = 0x%02" PRIx8 "   (%5" PRIu8 ")\n", testResult(s8, knownVal), s8, s8);

    s16 = bsdChecksum16(buffer, size8 / 2); // padding if not even?
    printf("%c bsdChecksum16 = 0x%04" PRIx16 " (%5" PRIu16 ")\n", testResult(s16, knownVal), s16, s16);

    s8 = sum8(buffer, size8);
    printf("%c         sum8  = 0x%02" PRIx8 "   (%5" PRIu8 ")\n", testResult(s8, knownVal), s8, s8);
    /*
    //Damm
    unsigned int input[4] = {5, 7, 2, 4};
    puts(damm(input, 4) ? "Checksum correct" : "Checksum incorrect");
    //-----
    */
    free(buffer);
}

const char progname[] = "chsum";

struct arg_lit *demo, *help;
struct arg_int *size, *val;
struct arg_file *file;
struct arg_end *end;

int main(int argc, char *argv[]) {
    /* the global arg_xxx structs are initialised within the argtable */
    void *argtable[] = {
            help = arg_litn(NULL, "help", 0, 1, "display this help and exit"),
            demo = arg_litn("d", "demo", 0, 1, "create demo file (all values"),
            size = arg_intn("s", NULL, "<n>", 0, 1, "size of demo file in dwords (default 4)"),
            val = arg_intn("v", NULL, "<n>", 0, 1, "known value of checksum"),
            file = arg_filen(NULL, NULL, "<file>", 1, 1, "input file"),
            end = arg_end(20),
    };

    int nerrors = arg_parse(argc, argv, argtable);

    int exitcode = 0;
    /* special case: '--help' takes precedence over error reporting */
    if (help->count > 0) {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        printf("Calculate different checksums of given file.\n\n");
        arg_print_glossary(stdout, argtable, "  %-15s %s\n");
        exitcode = EXIT_SUCCESS;
        goto exit;
    }

    /* If the parser returned any errors then display them and exit */
    if (nerrors > 0) {
        /* Display the error details contained in the arg_end struct.*/
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        exitcode = EXIT_FAILURE;
        goto exit;
    }

    // all params are OK, so do some real work:
    if (demo->count > 0) {
        if (val->count > 0)
            printf("Option '-v' is ignored when creating demo file.\n");
        createDemoFile(file->filename[0], (size->count > 0) ? size->ival : NULL);
        printf("Demo file '%s' has been created.\n", file->filename[0]);
    } else {
        if (size->count > 0)
            printf("Option '-s' is ignored when calculating checksums.\n");
        calculateAllChecksums(file->filename[0], (val->count > 0) ? val->ival : NULL);
    }

    exit:
    /* deallocate each non-null entry in argtable[] */
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return exitcode;
}

// **1** inspired or copied from https://github.com/lammertb/libcrc
