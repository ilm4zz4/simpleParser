/*
 * File:		Parser.cc
 *
 * Contents:		Definition of the Parser file Object
 *
 * Description:		n.a.
 *
 * Contributors:	Michele Rosellini  
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <map>

#include "Parser.h"

Parser::Parser() {
	m_pFile = NULL;
	m_cSep = '0';

}

bool Parser::setParameter(const char* pszFileName, char cSep) {
	m_pFile = fopen(pszFileName, "r");
	if (m_pFile != NULL) {
		fseek(m_pFile, 0, SEEK_SET);

		m_cSep = cSep;
		if (m_cSep == 0)
			m_cSep = '=';
	}else{
		assert (m_pFile!=NULL);
	}

	return (m_pFile!=NULL)?true:false;
}


Parser::~Parser() {

	if (m_pFile != NULL) {
		int iRetLoc = fclose(m_pFile);
		if (iRetLoc != 0) {
			//TODO; Eccezione!!!!
		}
	}
}

int Parser::getBool(const char* pszSection, const char* pszKey, bool* pbVal) {

	int iRet; /* return variable for function error code */
	int iRetLoc; /* local function call return value */
	char szBuff[LINE_LEN_MAX];
	int iVal;

	/* automatic variables inizialization */
	iVal = -1;

	iRet = _KeyValGet(pszSection, pszKey, szBuff, sizeof(szBuff));
	if (iRet == ERR_NONE) {
		ToLower(szBuff);
		iRetLoc = strcmp(szBuff, "false");
		if (iRetLoc == 0) {
			*pbVal = false;
			iVal = 0;
		} else {
			iRetLoc = strcmp(szBuff, "true");
			if (iRetLoc == 0) {
				*pbVal = true;
				iVal = 0;
			}
		}

		if (iVal != 0) {
			iVal = atoi(szBuff);

			if (iVal == 0)
				*pbVal = false;
			else
				*pbVal = true;
		}
	}

	return iRet;
}

int Parser::getDouble(const char* pszSection, const char* pszKey,
		double* pdVal) {
	int iRet; /* return variable for function error code */
	char szBuff[LINE_LEN_MAX];

	iRet = _KeyValGet(pszSection, pszKey, szBuff, sizeof(szBuff));
	if (iRet == ERR_NONE) {
		*pdVal = atof(szBuff);
	}

	return iRet;
}

int Parser::getInt(const char* pszSection, const char* pszKey, int* piVal) {
	int iRet; /* return variable for function error code */
	char szBuff[LINE_LEN_MAX];

	iRet = _KeyValGet(pszSection, pszKey, szBuff, sizeof(szBuff));
	if (iRet == ERR_NONE) {
		*piVal = atoi(szBuff);
	}

	return iRet;
}

int Parser::_KeyValGet(const char* pszSection, const char* pszKey,
		char* pszBuff, size_t stLen) {
	int iRet; /* return variable for function error code */

	iRet = SYS_ERR_PROC;

	iRet = _SectionFind(m_pFile, pszSection);
	if (iRet == ERR_NONE) {
		iRet = _KeyFind(m_pFile, m_cSep, pszKey, pszBuff, stLen);
	}

	return iRet;
}

int Parser::_KeyFind(FILE* pFile, char cSep, const char* pszKey, char* pszBuff,
		size_t stLen) {

	int iRet; /* return variable for function error code */
	int iRetLoc; /* local function call return value */
	bool bFail; /* function intermediate flag status */
	char* pszStr;
	char* pszVal;
	char* pszSection;
	char szLine[LINE_LEN_MAX];
	char szKey[LINE_LEN_MAX];

	/* defensive initialization */
	iRet = XUCFG_ERR_KEY;
	bFail = true;

	pszVal = NULL;

	strncpy(szKey, pszKey, sizeof(szKey));

	/* scan every line searcing a section template [xxx] */
	while (fgets(szLine, LINE_LEN_MAX, pFile) != NULL) {
		pszStr = Trim(szLine);

		if (_IsComment(pszStr) == false) {
			/* check if next section is reached */
			pszVal = _IsSection(pszStr);
			if (pszVal == NULL) {
				/* check if this string is a key */
				pszVal = strchr(pszStr, cSep);
				if (pszVal != NULL) {
					*pszVal = 0;
					pszVal++;

					pszStr = Trim(pszStr);
					ToLower(pszStr);
					ToLower(szKey);

					/* check if is the key searched */
					if (strcmp(pszStr, szKey) == 0) {
						bFail = false;
						break;
					}
				}
			} else {
				break;
			}
		}
	}

	if (bFail == false) {
		bFail = true;

		pszVal = Trim(pszVal);

		/* check if is a reference link (without 0 end) */
		iRetLoc = strncmp(pszVal, LINK_REF_MARK, strlen(LINK_REF_MARK));
		if (iRetLoc == 0) {
			/* defensive initialization */
			iRet = XUCFG_ERR_REFLINK;

			/* get the section */
			pszVal += strlen(LINK_REF_MARK);
			pszSection = strtok(pszVal, ":");
			if (pszSection != NULL) {
				/* search for section */
				iRetLoc = _SectionFind(pFile, pszSection);
				if (iRetLoc == ERR_NONE) {
					/* get the key */
					pszKey = strtok(NULL, "");

					/* search for key */
					iRetLoc = _KeyFind(pFile, cSep, pszKey, szLine,
							sizeof(szLine));
					if (iRetLoc == ERR_NONE) {
						pszVal = szLine;
						bFail = false;
					}
				}
			}
		} else {
			bFail = false;
		}
	}

	if (bFail == false) {
		/* return the val of the key */
		strncpy(pszBuff, pszVal, stLen - 1);
		pszBuff[stLen - 1] = 0;
		iRet = ERR_NONE;
	}

	return iRet;
}

int Parser::_SectionFind(FILE* pFile, const char* pszSection) {
	int iRet; /* return variable for function error code */
	int iRetLoc; /* local function call return value */
	char* pszStr;
	char szLine[LINE_LEN_MAX];
	char szSection[LINE_LEN_MAX];

	/* defensive initialization */
	iRet = XUCFG_ERR_FREAD;

	/* restore file pointer to the start of file */
	iRetLoc = fseek(pFile, 0, SEEK_SET);

	if (pszSection != NULL) {
		if (iRetLoc == 0) {
			/* defensive initialization */
			iRet = XUCFG_ERR_SECTION;

			strncpy(szSection, pszSection, sizeof(szSection));

			/* scan every line searcing a section template [xxx] */
			while (fgets(szLine, LINE_LEN_MAX, pFile) != NULL) {
				pszStr = Trim(szLine);

				if (_IsComment(pszStr) == false) {
					pszStr = _IsSection(pszStr);
					if (pszStr != NULL) {
						ToLower(pszStr);
						ToLower(szSection);
						if (strcmp(pszStr, szSection) == 0) {
							iRet = ERR_NONE;
							break;
						}
					}
				}
			}
		}
	} else {
		if (iRetLoc == 0)
			iRet = ERR_NONE;
	}

	return iRet;
}

int Parser::getStr(const char* pszSection, const char* pszKey, char* pszBuff,
		size_t stLen) {
	int iRet; /* return variable for function error code */
	char szBuff[LINE_LEN_MAX];

	iRet = _KeyValGet(pszSection, pszKey, szBuff, sizeof(szBuff));
	if (iRet == ERR_NONE) {
		strncpy(pszBuff, szBuff, stLen);
		szBuff[stLen - 1] = 0;
	}

	return iRet;
}
/* --------------------------------------------------  */
char* Parser::ToLower(char* pszString) {
	char* pcVal;

	pcVal = pszString;

	while (*pcVal != 0) {
		*pcVal = (char) tolower(*pcVal);
		pcVal++;
	}

	return pszString;
}

char* Parser::Trim(char* pszString) {
	size_t stLen;
	char* pcVal;

	if (pszString == NULL)
		return NULL;

	stLen = strlen(pszString);
	if (stLen == 0)
		return pszString;

	/* remove blank, line feed and carriage return from the tail */
	pcVal = &pszString[stLen - 1];

	while (pcVal > pszString) {
		if ((*pcVal == ' ') || (*pcVal == '\t') || (*pcVal == 0xd)
				|| (*pcVal == 0xa))
			pcVal--;
		else
			break;
	}

	pcVal++;
	*pcVal = 0;

	pcVal = pszString;
	/* remove blank, line feed and carriage return from the top */
	while (*pcVal != 0) {
		if ((*pcVal == ' ') || (*pcVal == '\t') || (*pcVal == 0xd)
				|| (*pcVal == 0xa))
			pcVal++;
		else
			break;
	}

	return pcVal;
}

bool Parser::_IsComment(char* pszLine) {
	bool bRet;

	bRet = false;

	pszLine = Trim(pszLine);

	if ((*pszLine == '#') || (*pszLine == ';'))
		bRet = true;

	return bRet;
}

char* Parser::_IsSection(char* pszLine) {
	char* pszRet;

	/* defensive initialization */
	pszRet = NULL;

	/* minimum len to make a section "[x]" */
	if (strlen(pszLine) >= 3) {
		if (pszLine[0] == '[') {
			if (pszLine[strlen(pszLine) - 1] == ']') {
				pszLine[strlen(pszLine) - 1] = 0;
				*pszLine = 0;
				pszRet = pszLine;
				pszRet++;
			}
		}
	}

	return pszRet;
}
