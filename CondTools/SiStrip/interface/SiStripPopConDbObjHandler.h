#ifndef SISTRIPPOPCON_DB_HANDLER_H
#define SISTRIPPOPCON_DB_HANDLER_H

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "CondCore/PopCon/interface/PopConSourceHandler.h"
#include "CondCore/DBCommon/interface/TagInfo.h"
#include "CondCore/DBCommon/interface/LogDBEntry.h"

#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <typeinfo>

namespace popcon{
  
  template <typename T, typename U>
    class SiStripPopConDbObjHandler : public popcon::PopConSourceHandler<T>{
    public:

    enum DataType { UNDEFINED=0, _Cabling=1, _Pedestal=2, _Noise=3, _Threshold=4, _BadStrip=5};

    //---------------------------------------
    //
    SiStripPopConDbObjHandler(const edm::ParameterSet& pset):
      m_name(pset.getUntrackedParameter<std::string>("name","SiStripPopConDbObjHandler")),
      m_since(pset.getUntrackedParameter<uint32_t>("since",5)),
      m_debugMode(pset.getUntrackedParameter<bool>("debug",false)){}; 

    //---------------------------------------
    //
    ~SiStripPopConDbObjHandler(){}; 

    //---------------------------------------
    //
    void getNewObjects(){
      edm::LogInfo   ("SiStripPopConDbObjHandler") << "[getNewObjects] for PopCon application " << m_name;
     
      if (m_debugMode){
	std::stringstream ss;
	ss << "\n\n------- " << m_name 
	   << " - > getNewObjects\n"; 
	if (this->tagInfo().size){
	  //check whats already inside of database
	  ss << "got offlineInfo"<<
	    this->tagInfo().name << ", size " << this->tagInfo().size << " " << this->tagInfo().token 
	     << " , last object valid since " 
	     << this->tagInfo().lastInterval.first << " token "   
	     << this->tagInfo().lastPayloadToken << "\n\n UserText " << this->userTextLog() 
	     << "\n LogDBEntry \n" 
	     << this->logDBEntry().logId<< "\n"
	     << this->logDBEntry().destinationDB<< "\n"   
	     << this->logDBEntry().provenance<< "\n"
	     << this->logDBEntry().usertext<< "\n"
	     << this->logDBEntry().iovtag<< "\n"
	     << this->logDBEntry().iovtimetype<< "\n"
	     << this->logDBEntry().payloadIdx<< "\n"
	     << this->logDBEntry().payloadName<< "\n"
	     << this->logDBEntry().payloadToken<< "\n"
	     << this->logDBEntry().payloadContainer<< "\n"
	     << this->logDBEntry().exectime<< "\n"
	     << this->logDBEntry().execmessage<< "\n"
	     << "\n\n-- user text " << this->logDBEntry().usertext.substr(this->logDBEntry().usertext.find_last_of("@")) ;
	} else {
	  ss << " First object for this tag ";
	}
	edm::LogInfo   ("SiStripPopConDbObjHandler") << ss.str();
      }
      if (isTransferNeeded())
	setForTransfer();
  
      edm::LogInfo   ("SiStripPopConDbObjHandler") << "[getNewObjects] for PopCon application " << m_name << " Done\n--------------\n";
    }


    //---------------------------------------
    //
    std::string id() const { return m_name;}

    private:
    //methods
    
    DataType getDataType(){
      
      if(typeid(T)==typeid(SiStripFedCabling)){
	edm::LogInfo   ("SiStripPopConDbObjHandler") << "[getDataType] for PopCon application " << m_name << " " << typeid(T).name();
	return _Cabling;
      }
      return UNDEFINED;
    }
    

    
    //---------------------------------------
    //
    bool isTransferNeeded(){


      edm::LogInfo   ("SiStripPopConDbObjHandler") << "[isTransferNeeded] checking for transfer"  << std::endl;
      std::stringstream ss_logdb, ss;
      std::stringstream ss1; 

      //get log information from previous upload
      if (this->tagInfo().size)
	ss_logdb << this->logDBEntry().usertext.substr(this->logDBEntry().usertext.find_last_of("@"));
      else
	ss_logdb << "";
      
      //get current config DB parameter
      const SiStripDbParams& dbParams = condObjBuilder->dbParams();

      condObjBuilder->getMetaDataString(ss);
       
      if (!strcmp(ss.str().c_str(),ss_logdb.str().c_str())){
	//string are equal, no need to do transfer
	edm::LogInfo   ("SiStripPopConDbObjHandler") 
	  << "[isTransferNeeded] the selected conditions are already uploaded in the last iov ("  
	  << this->tagInfo().lastInterval.first << ") open for the object " 
	  << this->logDBEntry().payloadName << " in the db " 
	  << this->logDBEntry().destinationDB << " parameters: "  << ss.str() << "\n NO TRANSFER NEEDED";
	return false;
      }
      this->m_userTextLog = ss.str();
      edm::LogInfo   ("SiStripPopConDbObjHandler") 
	<< "[isTransferNeeded] the selected conditions will be uploaded: " << ss.str() 
	<< "\n A- "<< ss.str()  << "\n B- " << ss_logdb.str() << "\n Fine";

      return true;
    }


    //---------------------------------------
    //
    void setForTransfer(){
      edm::LogInfo   ("SiStripPopConDbObjHandler") << "[setForTransfer] " << m_name << " getting data to be transferred "  << std::endl;
      
      T *obj=0; 
      condObjBuilder->getObj(obj);
 
      if(!this->tagInfo().size)
	m_since=1;
      else
	if (m_debugMode)
	  m_since=this->tagInfo().lastInterval.first+1; 

      if (obj!=0){

	edm::LogInfo   ("SiStripPopConDbObjHandler") <<"setting since = "<< m_since <<std::endl;
	this->m_to_transfer.push_back(std::make_pair(obj,m_since));
      }else{
	edm::LogError   ("SiStripPopConDbObjHandler") <<"[setForTransfer] " << m_name << "  : NULL pointer of obj " << typeid(T).name() << " reported by SiStripCondObjBuilderFromDb\n Transfer aborted"<<std::endl;
      }
    }

    private: 
    // data members
    std::string m_name;
    unsigned long long m_since;
    bool m_debugMode;
    edm::Service<U> condObjBuilder;
  };
}

#endif //SISTRIPPOPCON_DB_HANDLER_H
