//
//  Release.swift
//  Clover
//
//  Created by vector sigma on 19/10/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Foundation

let releasesUrl = URL(string:
"https://github.com/CloverHackyColor/CloverBootloader/releases/latest")!

func getLatestRelease(reply: @escaping (String?, String?) -> Void) {
  
  var link : String = "?"
  var vers : String = link
  
  var request = URLRequest(url: releasesUrl)
  request.httpMethod = "GET"

  let config = URLSessionConfiguration.default
  let session = URLSession(configuration: config)
  
  let task = session.dataTask(with: request, completionHandler: {(d, r, e) in
    //print(String(data: data!, encoding: .utf8)!)
    guard let data = d else {
      reply(link, vers)
      return
    }
    
    let reponse : String = String(data: data, encoding: .utf8)!
    for line in reponse.components(separatedBy: .newlines) {
      if (line.range(of: "href=\"/CloverHackyColor/CloverBootloader/releases/download/") != nil) {
        link = line.components(separatedBy: "href=\"")[1]
        link = "https://github.com\(link.components(separatedBy: "\"")[0])"
        //print(link)
        if link.lastPath.hasPrefix("CloverV2") && link.hasSuffix(".zip") {
          vers = link.components(separatedBy: "/releases/download/")[1]
          vers = vers.components(separatedBy: "/")[0]
          reply(link, vers)
          break
        }
        
      }
    }
  });
  
  task.resume()
  
  reply(link, vers)
}
