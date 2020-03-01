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

func getLatestReleases(reply: @escaping (String?, String?, String?, String?) -> Void) {
  var bootlink : String? = nil
  var bootvers : String? = nil
  var applink  : String? = nil
  var appvers  : String? = nil
  
  var request = URLRequest(url: releasesUrl)
  request.httpMethod = "GET"

  let config = URLSessionConfiguration.default
  let session = URLSession(configuration: config)
  
  let task = session.dataTask(with: request, completionHandler: {(d, r, e) in
    guard let data = d else {
      reply(bootlink, bootvers, applink, appvers)
      return
    }
    
    let reponse : String = String(data: data, encoding: .utf8)!
    for line in reponse.components(separatedBy: .newlines) {
      if (line.range(of: "href=\"/CloverHackyColor/CloverBootloader/releases/download/") != nil) {
        bootlink = line.components(separatedBy: "href=\"")[1]
        bootlink = "https://github.com\(bootlink!.components(separatedBy: "\"")[0])"
        
        if bootlink!.lastPath.hasPrefix("CloverV2") && bootlink!.hasSuffix(".zip") {
          bootvers = bootlink!.components(separatedBy: "/releases/download/")[1]
          bootvers = bootvers!.components(separatedBy: "/")[0]
          break
        }
      }
      
      if bootvers != nil && bootvers!.count == 0 {
        bootlink = nil
        bootvers = nil
      }
    }
    
    for line in reponse.components(separatedBy: .newlines) {
      if (line.range(of: "href=\"/CloverHackyColor/CloverBootloader/releases/download/") != nil) {
        applink = line.components(separatedBy: "href=\"")[1]
        //applink = "/CloverHackyColor/CloverBootloader/releases/download/5099/Clover.app_v1.17_r5104.pkg.zip\" rel=\"nofollow\" class=\"d-flex flex-items-center min-width-0\">"
        applink = "https://github.com\(applink!.components(separatedBy: "\"")[0])"
   
        if applink!.lastPath.hasPrefix("Clover.app_v") && applink!.hasSuffix(".zip") {
          // Clover.app_v1.17_r5104.pkg.zip
          appvers = applink!.components(separatedBy: "Clover.app_v")[1]
          //print(appvers)
          appvers = appvers!.components(separatedBy: "_r")[0]
          break
        }
      }
      
      if appvers != nil && appvers!.count == 0 {
        applink = nil
        appvers = nil
      }
    }
    /*
    print("bootlink: \(bootlink)")
    print("bootvers: \(bootvers)")
    print("applink:  \(applink)")
    print("appvers:  \(appvers)")
    */
    reply(bootlink, bootvers, applink, appvers)
  })
  
  task.resume()
}
