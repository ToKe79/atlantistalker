const char *CVS_ID_ATL_XML_CPP="$Id: atl-xml.cpp 704 2009-02-12 23:31:48Z wolcano $";
const char *CVS_REV_ATL_XML_CPP="$Rev: 704 $";

#include <libxml/xmlreader.h>

#include "atl-head.h"
#include "atl-xml.h"
#include "atl-base64.h"


int create_user_xml(UR_OBJECT user)
{
	if (user != NULL) {
		user->xml_buffer = xmlBufferCreate();
		xmlBufferSetAllocationScheme(user->xml_buffer, XML_BUFFER_ALLOC_DOUBLEIT);
		user->xml_writer = xmlNewTextWriterMemory(user->xml_buffer, 0);
		xmlTextWriterStartElement(user->xml_writer, BAD_CAST "data");
		return 0;
	}

	return 1;
}

int destruct_user_xml(UR_OBJECT user)
{
	if (user != NULL) {
		/* destruct writer first! */
		xmlFreeTextWriter(user->xml_writer);
		xmlBufferFree(user->xml_buffer);
		user->xml_writer = NULL;
		user->xml_buffer = NULL;
	}

	return 0;
}

int reset_user_xml(UR_OBJECT user)
{
	if ((user != NULL) && (user->xml_buffer)) {
		destruct_user_xml(user);
		create_user_xml(user);
		return 0;
	}

	return 1;
}

int reset_all_users_xml()
{
	UR_OBJECT u = user_first;

	while (u) {
		reset_user_xml(u);
		u = u->next;
	}

	return 0;
}

int user_shrink_xml(UR_OBJECT user)
{
	if (user != NULL) {
	}
	return 0;
}

int dump_user_data(UR_OBJECT user)
{
	if (user != NULL) {
		xmlTextWriterEndElement(user->xml_writer); /* uzatvarame documentelement */

		user_shrink_xml(user);

		switch (user->output_format) {
			case OUTPUT_FORMAT_XML:
				return dump_user_data_xml(user);
				break;
			case OUTPUT_FORMAT_PLAIN:
			default:
				return dump_user_data_plain(user);
		}
		reset_user_xml(user);
	}

	return -1;
}

/* vypise userovi XML parsovatelne data */
int dump_user_data_xml(UR_OBJECT user)
{
	if ((user != NULL) && (user->xml_writer != NULL)) {
		xmlTextWriterEndElement(user->xml_writer); /* uzavrieme 'data' document element */
		xmlTextWriterFlush(user->xml_writer);
		write2sock_ex(user, user->socket, (char *) (user->xml_buffer->content), strlen((const char *) (user->xml_buffer->content)));
		reset_user_xml(user);
		return 0;
	}

	return -1;
}

/* vypise userovi cisty text vhodny pre terminal */
int dump_user_data_plain(UR_OBJECT user)
{
	const xmlChar *s;
	xmlTextReaderPtr reader = NULL;

	if ((user != NULL) && (user->xml_writer != NULL)) {
		/* tento by sa dal reusovat, momentalne ale neoptimalizujeme */
		xmlTextWriterFlush(user->xml_writer);

		printf("'%s'\n", (const char *) (user->xml_buffer->content));

		reader = xmlReaderForMemory((const char *) (user->xml_buffer->content), strlen((const char *) (user->xml_buffer->content)), NULL, NULL, XML_PARSE_RECOVER);
		if (reader == NULL) {
			printf("dump_user_data_plain: Error creating reader\n");
			return 0;
		}
		/* precitame cele xml */
		/* xmlTextReaderExpand(reader); */

		while (xmlTextReaderRead(reader) == 1) {
			if (!xmlTextReaderIsEmptyElement(reader) && xmlTextReaderHasValue(reader)) {
				s = xmlTextReaderConstValue(reader);

				if (s != NULL) {
					size_t count;
					size_t count2;
					char *s2;

					count = strlen((char *)s);
					s2 = (char *) malloc(sizeof(char) * count + 1);
					memset((void *) s2, '\0', sizeof(char) * count + 1);
					count2 = decode_base64((unsigned char *) s2, (char *) s);
					s2[count2] = '\0';

					write2sock_ex(user, user->socket, s2, count2);
					free(s2);
				}
			}
		}

		xmlFreeTextReader(reader);
		reset_user_xml(user);
	}
	return -1;
}

int dump_all_users_data()
{
	UR_OBJECT u = user_first;

	while (u) {
		dump_user_data(u);
		u = u->next;
	}

	return 0;
}

