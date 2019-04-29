//
//  test.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 4/15/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

// run tests over classes

#include "PBFTPeerTest.hpp"
#include "test.hpp"

int RunTest(std::string filepath, std::string testOption){
    if(testOption == "all"){
        RunPBFT_Tests(filepath);
    }else if(testOption == "pbft"){
        RunPBFT_Tests(filepath);
    }

    return 0;
}


