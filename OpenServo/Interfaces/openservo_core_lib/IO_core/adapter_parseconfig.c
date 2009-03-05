/***************************************************************************
 *   Copyright (C) 2008 by Barry Carter,,,   *
 *   barry.carter@robotfuzz.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/* Reads a config file with a comma seperated value of the adapter name,
 * and the adapter lib location.
 * eg    Dummy Driver, libadapterdummy.so
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "IO_error.h"
#include "adapter_parseconfig.h"

/** Read one line out of the config file */
int config_read_line(FILE *file_handle, char *output)
{
    /* Read the line */
    fgets(output, 1024, file_handle);

    /* Error reading return the error number */
    if (ferror(file_handle))
        return -errno;                   /* Convert code to negative number */

    /* End of file reached */
    if (feof(file_handle))  
        return 0;

    return ERR_SUCCESS;
}

/** open the file, and return the file handle */
FILE *config_open(char *file_name)
{
    FILE *file_handle;

    file_handle = fopen(file_name, "r");
    /* Could not open file */
    if (file_handle == NULL )
    {
        printf("Error opening %s: %s (%u)\n", file_name, strerror(errno), errno);
        return NULL;
    }
    return file_handle;
}

/** Close the config file */
int config_close(FILE *file_handle)
{
    fclose(file_handle);
}

/** Parse one line of config file for the adapter name and library name */
int config_parse_line(char *config_line, char *adapter_name, char *adapter_lib)
{
    const char *config_ptr = config_line;
    char field[32];
    int n;
    int field_number = 0;

    // Use a regex to find the comma as the delimiter
    while (sscanf(config_ptr, "%31[^,]%n", field, &n) == 1 )
    {
        /* Strip all whitespace */
        strip(field);
        if (field[0] == '\n')
        {
            return ERR_FILE_GARBAGE;
        }
        /* Check which field numer we are on */
        if (field_number == 0)
            sprintf(adapter_name, "%s", field);
        else
        {
            field[strlen(field)-1] = '\0';
            sprintf(adapter_lib, "%s", field);
        }
        /* advance the pointer by the number of characters read */
        config_ptr += n;
        if ( *config_ptr != ',' )
        {
            break; /* didn't find an expected delimiter, done? */
        }
        /* skip the delimiter */
        ++config_ptr;
        field_number = 1;
    }
    return ERR_SUCCESS;
}

/** Strip white space from the line */
void strip(char *buf)
{
    char tmpbuf[1024];
    char *tmpbufp;
    int n = 0;

    /* Pointer to the head of the buffer */
    tmpbufp = &tmpbuf[0];

    /* Copy the incoming buffer to a backup */
    memcpy(tmpbuf, buf, strlen(buf)+1);

    /* Loop through the list looking for whitespace using incremental pointers */
    while (*tmpbufp == ' ')
    {
        *tmpbufp++;
        n++;
    }
    /* Copy the temp buffer into the original buffer */
    memcpy(buf, tmpbufp, strlen(tmpbuf) - n+1);
}

/** Parse the config file into the name and lib location */
int config_parse(FILE *file_handle, char *adapter_name, char *adapter_lib)
{
    char config_line[1024];

    // is it the end of the file?
    if (config_read_line(file_handle, config_line) == 0)
    {
        return 0;
    }

    if (config_parse_line(config_line, adapter_name, adapter_lib) == ERR_FILE_GARBAGE)
    {
        return ERR_FILE_GARBAGE;
    }
    return ERR_SUCCESS;
        
}
