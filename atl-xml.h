#ifndef _ATL_XML_H
#define _ATL_XML_H

#define CVS_ID_ATL_XML_H "$Id: atl-xml.h 704 2009-02-12 23:31:48Z wolcano $"
#define CVS_REV_ATL_XML_H "$Rev: 704 $"

#include <libxml/tree.h>
#include <libxml/parser.h>

int destruct_user_xml(UR_OBJECT);
int create_user_xml(UR_OBJECT);
int reset_user_xml(UR_OBJECT);
int reset_all_users_xml();
int dump_user_data(UR_OBJECT);
int dump_user_data_plain(UR_OBJECT);
int dump_user_data_xml(UR_OBJECT);
int dump_all_users_data();

#endif
