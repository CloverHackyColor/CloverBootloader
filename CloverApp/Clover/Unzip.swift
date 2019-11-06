//
//  Unzip.swift
//  Clover
//
//  Created by vector sigma on 31/10/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Foundation

func unzip(file: String,
           destination: String,
           success: @escaping (Bool) -> Void) {
  let task : Process = Process()
  if #available(OSX 10.13, *) {
    task.executableURL = URL(fileURLWithPath: "/usr/bin/unzip")
  } else {
    task.launchPath = "/usr/bin/unzip"
  }
  // unzip -d output_dir/ zipfiles.zip
  task.arguments = ["-d", "\(destination)/", file]
  
  let pipe: Pipe = Pipe()
  task.standardOutput = pipe
  task.standardError = pipe
  
  task.terminationHandler = { task in
    success((task.terminationStatus == 0))
  }
  
  task.launch()
}
