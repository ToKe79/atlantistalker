/*
 * Copyright (C), 2000-2007 by the monit project group.
 * All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ATL_BASE64_H
#define _ATL_BASE64_H

#define CVS_ID_ATL_BASE64_H "$Id: atl-base64.h 704 2009-02-12 23:31:48Z wolcano $"
#define CVS_REV_ATL_BASE64_H "$Rev: 704 $"

int   decode_base64(unsigned char *dest, const char *src);
char *encode_base64(int size, unsigned char *src);

#endif


