//
//  PBFT_Peer.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/19/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <string>
#include "PBFT_Peer.hpp"


PBFT_Peer::PBFT_Peer(std::string id) : Peer(id){
    _viewNumber = 0;
    _primary = "";
}

void PBFT_Peer::findPrimary(){
    int primaryIndex = _viewNumber%_groupMembers.size();
//    
//    _groupMembers.keySet()
//    
//    for(int i =0; i < .size; ){
//        
//    }
}
