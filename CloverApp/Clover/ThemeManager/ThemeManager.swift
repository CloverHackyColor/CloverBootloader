//
//  ThemeManager.swift
//  ThemeManager
//
//  Created by vector sigma on 04/01/2020.
//  Copyright © 2020 vectorsigma. All rights reserved.
//

import Cocoa

let kThemeUserKey = "themeUser"
let kThemeRepoKey = "themeRepo"

let kDefaultThemeUser = "CloverHackyColor" // CloverHackyColor
let kDefaultThemeRepo = "CloverThemes"

enum ThemeDownload {
  case indexOnly
  case thumbnail
  case complete
}

let kCloverThemeAttributeKey = "org.cloverTheme.sha"

final class ThemeManager: NSObject, URLSessionDataDelegate {
  var delegate : ThemeManagerVC?
  private var user : String
  private var repo : String
  var basePath : String
  private var urlBaseStr : String
  private var themeManagerIndexDir : String

  let userAgent = "Clover"
  
  required init(user: String, repo: String,
                basePath: String,
                indexDir : String,
                delegate: ThemeManagerVC?) {
    self.user = user
    self.repo = repo
    self.basePath = basePath
    self.urlBaseStr = "https://api.github.com/repos/\(user)/\(repo)/git/trees/master?recursive=1"
    self.themeManagerIndexDir = indexDir
    self.delegate = delegate
    if !fm.fileExists(atPath: self.themeManagerIndexDir) {
      try? fm.createDirectory(atPath: self.themeManagerIndexDir,
                              withIntermediateDirectories: true,
                              attributes: nil)
    }
  }
  
  public func getIndexedThemes() -> [String] {
    var themes = [String]()
    if self.getSha() != nil {
      let themesIndexPath = self.themeManagerIndexDir.addPath("Themes")
      if let files : [String] = try? fm.contentsOfDirectory(atPath: themesIndexPath) {
        for f in files {
          let theme : String = f.deletingFileExtension
          let ext : String = f.fileExtension
          if ext == "plist" {
            themes.append(theme)
          }
        }
      }
    }
    return themes.sorted()
  }
  
  public func getThemes(completion: @escaping ([String]) -> ()) {
    var themes : [String] = [String]()
    let themesIndexPath : String = self.themeManagerIndexDir.addPath("Themes")
    
    self.getInfo(urlString: urlBaseStr) { (success) in
     
      do {
        let files : [String] = try fm.contentsOfDirectory(atPath: themesIndexPath)
        for f in files {
          let theme : String = f.deletingFileExtension
          let ext : String = f.fileExtension
          if ext == "plist" {
            themes.append(theme)
          }
        }
        completion(themes)
      } catch {
        completion(themes)
      }
      
    }
  }
  
  /// Return the sha string only if the sha exist, and only if Themes directory exists.
  func getSha() -> String? {
    var sha : String? = nil
    let themesPath : String = "\(self.themeManagerIndexDir)/Themes"
    let shaPath : String = "\(self.themeManagerIndexDir)/sha"
    
    if fm.fileExists(atPath: themesPath) && fm.fileExists(atPath: shaPath) {
      if let data : Data = try? Data(contentsOf: URL(fileURLWithPath: shaPath)) {
        sha = String(data: data, encoding: .utf8)
        if ((sha != nil) && (sha!.count < 40)) {
          sha = nil
        }
      }
    }
    
    return sha
  }
  
  private func normalize(_ url: String) -> String {
    if (url.range(of: " ") != nil) {
      return url.replacingOccurrences(of: " ", with: "%20")
    }
    return url
  }
  
  private func downloadloadFile(at url: String, dst: String,
                                completion: @escaping (Bool) -> ()) {
    if let validURL : URL = URL(string: self.normalize(url)) {
      let upperDir : String = dst.deletingLastPath
      if !fm.fileExists(atPath: upperDir) {
        do {
          try fm.createDirectory(atPath: upperDir,
                                 withIntermediateDirectories: true,
                                 attributes: nil)
        } catch {
          print("DF1, \(error)")
          completion(false)
          return
        }
      }
      var request : URLRequest = URLRequest(url: validURL)
      request.httpMethod = "GET"
      request.setValue(userAgent, forHTTPHeaderField: "User-Agent")
      
      let config = URLSessionConfiguration.ephemeral
      let session = URLSession(configuration: config)
      
      let task = session.dataTask(with: request, completionHandler: {(d, r, e) in
        if (e != nil) {
          print("DF2, \(e!)")
          completion(false)
          return
        }
        guard let data = d else {
          print("DF3, Error: no datas.")
          completion(false)
          return
        }
        do {
          try data.write(to: URL(fileURLWithPath: dst))
          completion(true)
        } catch {
          print("DF4, \(error)")
          completion(false)
        }
      })
      
      task.resume()

    } else {
      print("DF5, Error: invalid url '\(url)'.")
      completion(false)
    }
  }

  private func getInfo(urlString: String, completion: @escaping (Bool) -> ()) {
    if let url = URL(string: self.normalize(urlString)) {
      var request : URLRequest = URLRequest(url: url)
      request.httpMethod = "GET"
      request.setValue("application/json; charset=utf-8", forHTTPHeaderField: "Content-Type")
      request.setValue(userAgent, forHTTPHeaderField: "User-Agent")
      let config = URLSessionConfiguration.ephemeral
      let session = URLSession(configuration: config)
      
      let task = session.dataTask(with: request, completionHandler: {(d, r, e) in
        if (e != nil) {
          print("GI1, \(e!)")
          completion(false)
          return
        }
        
        guard let data = d else {
          print("GI2, no data.")
          completion(false)
          return
        }
        
        guard let utf8 = String(decoding: data, as: UTF8.self).data(using: .utf8) else {
          print("GI3, data is not utf8.")
          completion(false)
          return
        }
        
        do {
          let json = try JSONSerialization.jsonObject(with: utf8, options: .mutableContainers)
       
          if let jdict = json as? [String: Any] {
            guard let truncated = jdict["truncated"] as? Bool else {
              print("GI4, Error: 'truncated' key not found")
              completion(false)
              return
            }
            
            if truncated == true {
              print("GI4, Error: json has truncated list.")
              completion(false)
              return
            }
            
            guard let sha = jdict["sha"] as? String else {
              print("GI4, Error: 'sha' key not found")
              completion(false)
              return
            }
            
            if sha == self.getSha() {
              // we're done because we already have all the files needed
              completion(false)
              return
            }
            
            guard let tree = jdict["tree"] as? [[String: Any]] else {
              print("GI4, Error: 'tree' key not found, or not an array.")
              completion(false)
              return
            }
            
            let shaPath : String = "\(self.themeManagerIndexDir)/\(sha)"
            
            do {
              if !fm.fileExists(atPath: shaPath) {
                try fm.createDirectory(atPath: shaPath, withIntermediateDirectories: true, attributes: nil)
              }
              
            } catch {
              print("GI5, Error: can't write sha commit.")
              completion(false)
              return
            }
            
            for obj in tree {
              /*
              "path": ".gitignore",
              "mode": "100644",
              "type": "blob",
              "sha": "e43b0f988953ae3a84b00331d0ccf5f7d51cb3cf",
              "size": 10,
              "url": "https://api.github.com/repos/vectorsigma72/CloverThemes/git/blobs/e43b0f988953ae3a84b00331d0ccf5f7d51cb3cf"
              */
           
              guard let type = obj["type"] as? String else {
                print("GI6, Error: 'type' key not found")
                completion(false)
                break
              }
              
              if let path = obj["path"] as? String {
                
                if !path.hasPrefix(".") && type == "blob" { // .gitignore, .DS_Store
                  let themeName : String = path.components(separatedBy: "/")[0]
                  let plistPath : String = "\(self.themeManagerIndexDir)/\(sha)/\(themeName).plist"
                  let theme : NSMutableArray = NSMutableArray(contentsOfFile: plistPath) ?? NSMutableArray()
                  if !theme.contains(path) {
                    theme.add(path)
                  }
                  
                
                  if !theme.write(toFile: plistPath, atomically: false) {
                    print("GI7, Error: 'path' key not found.")
                    completion(false)
                    break
                  }
                }
              } else {
                print("GI8, Error: 'path' key not found.")
                completion(false)
                break
              }
            }
            
            // move temp files in standard position
            do {
              try sha.write(toFile: "\(self.themeManagerIndexDir)/sha", atomically: true, encoding: .utf8)
              if fm.fileExists(atPath: "\(self.themeManagerIndexDir)/Themes") {
                try fm.removeItem(atPath: "\(self.themeManagerIndexDir)/Themes")
              }
              try fm.moveItem(atPath: shaPath, toPath: "\(self.themeManagerIndexDir)/Themes")
              self.removeOld(new: sha)
              completion(true)
            } catch {
              print("GI9, \(error)")
              completion(false)
              return
            }
            
            //remove old themes (old sha)
          } else {
            print("GI10, json is not a dictionary (API change?).")
            completion(false)
          }
         
        } catch {
          print("GI11, \(error)")
          completion(false)
        }
      })
      
      task.resume()
    } else {
      print("GI12, \(urlString) is invalid.")
      completion(false)
    }
  }
  
  /// Remove old thumbnails from old sha commits
  private func removeOld(new sha: String) {
    do {
      let contents : [String] = try fm.contentsOfDirectory(atPath: self.themeManagerIndexDir)
      for c in contents {
        if c != "sha" && c != "Themes" && c != sha {
          try fm.removeItem(atPath: "\(self.themeManagerIndexDir)/\(c)")
        }
      }
    } catch { }
  }
  
  /// Return the path for a given theme, if the download succeded
  public func download(theme: String, down: ThemeDownload, completion: @escaping (String?) -> ()) {
    if let sha = getSha() {
      let shaPath : String = self.basePath.addPath(sha)
      let themeDest : String = (down == .complete)
        ? self.themeManagerIndexDir.addPath("Downloads").addPath(theme)
        :  shaPath.addPath(theme)

      if (down != .complete) && fm.fileExists(atPath: themeDest) {
        completion(themeDest)
      } else {
        if !fm.fileExists(atPath: themeDest) {
          do {
            try fm.createDirectory(atPath: themeDest,
                                   withIntermediateDirectories: true,
                                   attributes: nil)
          } catch {}
        }
 
        let plistPath : String = "\(themeManagerIndexDir)/Themes/\(theme).plist"

        if let files : [String] = NSArray(contentsOfFile: plistPath) as? [String] {
          let fc : Int = files.count
          if fc > 0 {
            var broken : Bool = false
            let dg = DispatchGroup()
            for i in 0..<fc {
              dg.enter()
              if broken {
                dg.leave()
                break
              } else {
                let file : String = files[i]
                // build the url
                let furl : String = "https://github.com/\(self.user)/\(self.repo)/raw/master/\(file)"
                
                if down == .thumbnail {
                  if file != theme.addPath("screenshot.png")
                    && file != theme.addPath("theme.svg")
                    && file != theme.addPath("theme.plist") {
                    dg.leave()
                    continue
                  }
                }
                
                let filedest : String = (down == .complete)
                  ? themeDest.deletingLastPath.addPath(file)
                  : shaPath.addPath(file)
                
                if !fm.fileExists(atPath: filedest) {
                  self.downloadloadFile(at: self.normalize(furl), dst: filedest) { (success) in
                    broken = (success == false)
                    dg.leave()
                  }
                } else {
                  dg.leave()
                }
              }
            }
            
            dg.notify(queue: .main) {
              if broken {
                completion(nil)
              } else {
                completion(themeDest)
              }
            }
            
          } else {
            completion(nil)
          }
        } else {
          completion(nil)
        }
      }
    } else {
      completion(nil)
    }
    
  }
  
  public func getImageUrl(for theme: String, completion: @escaping (String?) -> ()) {
    /*
     We are going to load an image in a web view,
     so no matter if the url will be a file on the local
     filesystem (downloaded theme) or just on the online repository.
     */
    if let sha : String = self.getSha() {
      let localTheme : String = "\(basePath)/\(sha)/\(theme)"
      let png : String = "\(localTheme)/screenshot.png"
      let svg : String = "\(localTheme)/theme.svg"
      if fm.fileExists(atPath: png) {
        completion(png)
        return
      } else if fm.fileExists(atPath: svg) {
        completion(svg)
        return
      }
    }
    
    
    // theme not found?? Downloading...
    self.download(theme: theme, down: .thumbnail) { (path) in
      if let localTheme : String = path {
        let png : String = "\(localTheme)/screenshot.png"
        let svg : String = "\(localTheme)/theme.svg"
        if fm.fileExists(atPath: png) {
          completion(png)
        } else if fm.fileExists(atPath: svg) {
          completion(svg)
        }
      } else {
        completion(nil)
      }
    }
  }
  
  public func signTheme(at path: String) {
    if let sha : String = getSha() {
      let fileURL : URL = URL(fileURLWithPath: path)
      let data : Data? = sha.data(using: .utf8)
      
      // remove all attributes
      do {
        let list = try fileURL.listExtendedAttributes()
        for attr in list {
          try fileURL.removeExtendedAttribute(forName: attr)
        }
      } catch {
        print(error.localizedDescription)
      }
      
      // set attribute
      do {
        try fileURL.setExtendedAttribute(data: data!, forName: kCloverThemeAttributeKey)
        /* nine test
        let adata = try fileURL.extendedAttribute(forName: attr)
        print(String(data: adata, encoding: .utf8)!)
        */
      } catch let error {
        print(error.localizedDescription)
      }
    }
  }
  
  public func exist(theme: String) -> Bool {
    return fm.fileExists(atPath: "\(self.themeManagerIndexDir)/Themes/\(theme).plist")
  }

}
