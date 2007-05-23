//===--- Decl.cpp - Declaration AST Node Implementation -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by Chris Lattner and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Decl class and subclasses.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/Decl.h"
#include "clang/Lex/IdentifierTable.h"
using namespace llvm;
using namespace clang;

// temporary statistics gathering
static unsigned nFuncs = 0;
static unsigned nBlockVars = 0;
static unsigned nFileVars = 0;
static unsigned nParmVars = 0;
static unsigned nSUE = 0;
static unsigned nEnumConst = 0;
static unsigned nEnumDecls = 0;
static unsigned nTypedef = 0;
static unsigned nFieldDecls = 0;
static bool StatSwitch = false;

bool Decl::CollectingStats(bool enable) {
  if (enable) StatSwitch = true;
	return StatSwitch;
}

void Decl::PrintStats() {
    fprintf(stderr, "*** Decl Stats:\n");
		fprintf(stderr, "  %d decls total.\n", 
		  nFuncs+nBlockVars+nFileVars+nParmVars+nFieldDecls+nSUE+nEnumDecls+nEnumConst+nTypedef);
    fprintf(stderr, "    %d function decls, %d each (%d bytes)\n", 
		  nFuncs, sizeof(FunctionDecl), nFuncs*sizeof(FunctionDecl));
    fprintf(stderr, "    %d block variable decls, %d each (%d bytes)\n", 
		  nBlockVars, sizeof(BlockVarDecl), nBlockVars*sizeof(BlockVarDecl));
    fprintf(stderr, "    %d file variable decls, %d each (%d bytes)\n", 
		  nFileVars, sizeof(FileVarDecl), nFileVars*sizeof(FileVarDecl));
    fprintf(stderr, "    %d parameter variable decls, %d each (%d bytes)\n", 
		  nParmVars, sizeof(ParmVarDecl), nParmVars*sizeof(ParmVarDecl));
    fprintf(stderr, "    %d field decls, %d each (%d bytes)\n", 
		  nFieldDecls, sizeof(FieldDecl), nFieldDecls*sizeof(FieldDecl));
    fprintf(stderr, "    %d struct/union/enum decls, %d each (%d bytes)\n", 
		  nSUE, sizeof(RecordDecl), nSUE*sizeof(RecordDecl));
    fprintf(stderr, "    %d enum decls, %d each (%d bytes)\n", 
		  nEnumDecls, sizeof(EnumDecl), nEnumDecls*sizeof(EnumDecl));
    fprintf(stderr, "    %d enum constant decls, %d each (%d bytes)\n", 
		  nEnumConst, sizeof(EnumConstantDecl), nEnumConst*sizeof(EnumConstantDecl));
    fprintf(stderr, "    %d typedef decls, %d each (%d bytes)\n", 
		  nTypedef, sizeof(TypedefDecl), nTypedef*sizeof(TypedefDecl));
		fprintf(stderr, "Total bytes = %d\n", 
		  nFuncs*sizeof(FunctionDecl)+nBlockVars*sizeof(BlockVarDecl)+
      nFileVars*sizeof(FileVarDecl)+nParmVars*sizeof(ParmVarDecl)+
      nFieldDecls*sizeof(FieldDecl)+nSUE*sizeof(RecordDecl)+
      nEnumDecls*sizeof(EnumDecl)+nEnumConst*sizeof(EnumConstantDecl)+
			nTypedef*sizeof(TypedefDecl));
}

void Decl::addDeclKind(const Kind k) {
	switch (k) {
	case Typedef:
		nTypedef++;
		break;
	case Function:
		nFuncs++;
		break;
	case BlockVariable:
		nBlockVars++;
		break;
	case FileVariable:
		nFileVars++;
		break;
	case ParmVariable:
		nParmVars++;
		break;
	case EnumConstant:
		nEnumConst++;
		break;
	case Field:
		nFieldDecls++;
		break;
	case Struct:
	case Union:
	case Class:
		nSUE++;
		break;
	case Enum:
		nEnumDecls++;
		break;
	}
}

// Out-of-line virtual method providing a home for Decl.
Decl::~Decl() {
}

const char *Decl::getName() const {
  if (const IdentifierInfo *II = getIdentifier())
    return II->getName();
  return "";
}


FunctionDecl::~FunctionDecl() {
  delete[] ParamInfo;
}

unsigned FunctionDecl::getNumParams() const {
  return cast<FunctionTypeProto>(getType().getTypePtr())->getNumArgs();
}

void FunctionDecl::setParams(VarDecl **NewParamInfo, unsigned NumParams) {
  assert(ParamInfo == 0 && "Already has param info!");
  assert(NumParams == getNumParams() && "Parameter count mismatch!");
  
  // Zero params -> null pointer.
  if (NumParams) {
    ParamInfo = new VarDecl*[NumParams];
    memcpy(ParamInfo, NewParamInfo, sizeof(VarDecl*)*NumParams);
  }
}


/// defineElements - When created, EnumDecl correspond to a forward declared
/// enum.  This method is used to mark the decl as being defined, with the
/// specified contents.
void EnumDecl::defineElements(EnumConstantDecl **Elts, unsigned NumElts) {
  assert(!isDefinition() && "Cannot redefine enums!");
  setDefinition(true);
  NumElements = NumElts;
  if (NumElts) {
    Elements = new EnumConstantDecl*[NumElts];
    memcpy(Elements, Elts, NumElts*sizeof(Decl*));
  }
}



/// defineBody - When created, RecordDecl's correspond to a forward declared
/// record.  This method is used to mark the decl as being defined, with the
/// specified contents.
void RecordDecl::defineBody(FieldDecl **members, unsigned numMembers) {
  assert(!isDefinition() && "Cannot redefine record!");
  setDefinition(true);
  NumMembers = numMembers;
  if (numMembers) {
    Members = new FieldDecl*[numMembers];
    memcpy(Members, members, numMembers*sizeof(Decl*));
  }
}

FieldDecl* RecordDecl::getMember(IdentifierInfo *name) {
  if (Members == 0 || NumMembers < 0)
    return 0;
	
  // linear search. When C++ classes come along, will likely need to revisit.
  for (int i = 0; i < NumMembers; ++i) {
    if (Members[i]->getIdentifier() == name)
      return Members[i];
  }
  return 0;
}

