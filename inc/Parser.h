/*
 * File:		Parser.h
 *
 * Contents:		Definition of the Parser file Object
 *
 * Description:		n.a.
 *
 * Contributors:	Michele Rosellini  
 *
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <stdio.h>

#define  LINE_LEN_MAX  (255)

/* Private macros definition */
#define LINK_REF_MARK   "@>"

/* @Def  common error for all modules */
enum {
	ERR_NONE, /*no error*/
	SYS_ERR_PARAM, /* invalid input parameters            */
	SYS_ERR_INIT, /* module descriptor not initialized   */
	SYS_ERR_PROC, /* process error; unexpected condition */
	SYS_ERR_BOUNDARY, /* memory access out of boundary       */
	SYS_ERR_DATA, /* inconsistent data                   */

	XUCFG_ERR_FOPEN, /* unable to open the file */
	XUCFG_ERR_FCLOSE, /* fail on file close */
	XUCFG_ERR_FREAD, /* unable to read the file */
	XUCFG_ERR_SECTION, /* the section don't exist */
	XUCFG_ERR_KEY, /* the key don't exist */
	XUCFG_ERR_REFLINK, /* the reference for the key don't exist */

};

class Parser {

	FILE* m_pFile; /*File descriptor*/
	char m_cSep;  /*Separator*/
	bool _IsComment(char* pszLine);
	char* _IsSection(char* pszLine);
	char* ToLower(char* pszString);
	char* Trim(char* pszString);
	int _KeyFind(FILE* pFile, char cSep, const char* pszKey, char* pszBuff,size_t stLen);
	int _KeyValGet(const char* pszSection, const char* pszKey, char* pszBuff,size_t stLen);
	int _SectionFind(FILE* pFile, const char* pszSection);

public:
	Parser();
	~Parser();
	bool setParameter(const char* pszFileName, char cSep);
	int getBool(const char* pszSection, const char* pszKey, bool* pbVal);
	int getDouble(const char* pszSection, const char* pszKey, double* pdVal);
	int getInt(const char* pszSection, const char* pszKey, int* piVal);
	int getStr(const char* pszSection, const char* pszKey, char* pszBuff,
			size_t stLen);


};

#endif /* PARSER_H_ */
