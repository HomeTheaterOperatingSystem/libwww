/*		HTML Generator
**		==============
**
**	This version of the HTML object sends HTML markup to the output stream.
**
** Bugs:	Line wrapping is not done at all.
**		All data handled as PCDATA.
**		Should convert old XMP, LISTING and PLAINTEXT to PRE.
**
**	It is not obvious to me right now whether the HEAD should be generated
**	from the incomming data or the anchor.  Currently itis from the former
**	which is cleanest.
*/

/* Implements:
*/
#include "HTMLGen.h"

#include <stdio.h>
#include "HTMLDTD.h"
#include "HTStream.h"
#include "SGML.h"
#include "HTFormat.h"

#define PUTC(c) (*this->targetClass.put_character)(this->target, c)
#define PUTS(s) (*this->targetClass.put_string)(this->target, s)
#define PUTB(s,l) (*this->targetClass.write)(this->target, s, l)

/*		HTML Object
**		-----------
*/

struct _HTStream {
	CONST HTStreamClass *		isa;	
	HTStream * 			target;
	HTStreamClass			targetClass;	/* COPY for speed */
};

struct _HTStructured {
	CONST HTStructuredClass *	isa;
	HTStream * 			target;
	HTStreamClass			targetClass;	/* COPY for speed */
};


/*	Character handling
**	------------------
*/
PRIVATE void HTMLGen_put_character ARGS2(HTStructured *, this, char, c)
{
    PUTC(c);
}



/*	String handling
**	---------------
*/
PRIVATE void HTMLGen_put_string ARGS2(HTStructured *, this, CONST char*, s)
{
    PUTS(s);
}

PRIVATE void HTMLGen_write ARGS3(HTStructured *, this, CONST char*, s, int, l)
{
    PUTB(s,l);
}


/*	Start Element
**	-------------
*/
PRIVATE void HTMLGen_start_element ARGS4(
	HTStructured *, 	this,
	int,		element_number,
	BOOL*,	 	present,
	char **,	value)
{
    int i;

    HTTag * tag = &HTML_dtd.tags[element_number];
    PUTC('<');
    PUTS(tag->name);
    if (present) for (i=0; i< tag->number_of_attributes; i++) {
        if (present[i]) {
	    PUTC(' ');
	    PUTS(tag->attributes[i].name);
	    if (value[i]) {
	 	PUTS("=\"");
		PUTS(value[i]);
		PUTC('"');
	    }
	}
    }
    PUTC('>');
}


/*		End Element
**		-----------
**
*/
/*	When we end an element, the style must be returned to that
**	in effect before that element.  Note that anchors (etc?)
**	don't have an associated style, so that we must scan down the
**	stack for an element with a defined style. (In fact, the styles
**	should be linked to the whole stack not just the top one.)
**	TBL 921119
*/
PRIVATE void HTMLGen_end_element ARGS2(HTStructured *, this,
			int , element_number)
{
    PUTS("</");
    PUTS(HTML_dtd.tags[element_number].name);
    PUTC('>');
}


/*		Expanding entities
**		------------------
**
*/

PRIVATE void HTMLGen_put_entity ARGS2(HTStructured *, this, int, entity_number)
{
    PUTC('&');
    PUTS(HTML_dtd.entity_names[entity_number]);
    PUTC(';');
}



/*	Free an HTML object
**	-------------------
**
**	Note that the SGML parsing context is freed, but the created object is not,
**	as it takes on an existence of its own unless explicitly freed.
*/
PRIVATE void HTMLGen_free ARGS1(HTStructured *, this)
{
    (*this->targetClass.free)(this->target);	/* ripple through */
    free(this);
}



PRIVATE void HTMLGen_end_document ARGS1(HTStructured *, this)
{
    PUTC('\n');		/* Make sure ends with newline for sed etc etc */
    (*this->targetClass.end_document)(this->target);
}


PRIVATE void PlainToHTML_end_document ARGS1(HTStream *, this)
{
    PUTS("</PRE></BODY>\n");/* Make sure ends with newline for sed etc etc */
    (*this->targetClass.end_document)(this->target);
}



/*	Structured Object Class
**	-----------------------
*/
PUBLIC CONST HTStructuredClass HTMLGeneration = /* As opposed to print etc */
{		
	"text/html",
	HTMLGen_free,
	HTMLGen_end_document,
	HTMLGen_put_character, 	HTMLGen_put_string, HTMLGen_write,
	HTMLGen_start_element, 	HTMLGen_end_element,
	HTMLGen_put_entity
}; 


/*	Subclass-specific Methods
**	-------------------------
*/

PUBLIC HTStructured * HTMLGenerator ARGS1(HTStream *, output)
{
    HTStructured* this = (HTStructured*)malloc(sizeof(*this));
    if (this == NULL) outofmem(__FILE__, "HTMLGenerator");
    this->isa = &HTMLGeneration;       

    this->target = output;
    this->targetClass = *this->target->isa; /* Copy pointers to routines for speed*/

    return this;
}

/*	Stream Object Class
**	-------------------
**
**	Ok so this object just converts a plain text stream into HTML
*/
PUBLIC CONST HTStreamClass PlainToHTMLConversion = /* As opposed to print etc */
{		
	"plaintexttoHTML",
	HTMLGen_free,	
	PlainToHTML_end_document,	
	HTMLGen_put_character,
	HTMLGen_put_string,
	HTMLGen_write,
}; 


/*	HTConverter from plain text to HTML Stream
**	------------------------------------------
*/

PUBLIC HTStream* HTPlainToHTML ARGS3(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,	
	HTStream *,		sink)
{
    HTStream* this = (HTStream*)malloc(sizeof(*this));
    if (this == NULL) outofmem(__FILE__, "PlainToHTML");
    this->isa = &PlainToHTMLConversion;       

    this->target = sink;
    this->targetClass = *this->target->isa;
    	/* Copy pointers to routines for speed*/
	
    PUTS("<BODY>\n<PRE>\n");
    return this;
}


