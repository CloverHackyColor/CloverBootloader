//
//  Tasks.swift
//  Clover
//
//  Created by vector sigma on 24/11/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Foundation

func run(cmd: String) {
  let task = Process()
  if #available(OSX 10.13, *) {
    task.executableURL = URL(fileURLWithPath: "/bin/bash")
  } else {
    task.launchPath = "/bin/bash"
  }
  task.environment = ProcessInfo.init().environment
  task.arguments = ["-c", cmd]
  task.launch()
}

