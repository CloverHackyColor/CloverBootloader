//
//  ThemeManager.swift
//  ThemeManager
//
//  Created by vector sigma on 04/01/2020.
//  Copyright © 2020 vectorsigma. All rights reserved.
//

import Cocoa
import CommonCrypto

let kThemeInfoFile = ".CTv1_i"
let kThemeUserKey = "themeUser"
let kThemeRepoKey = "themeRepo"

let kDefaultThemeUser = "CloverHackyColor" // CloverHackyColor
let kDefaultThemeRepo = "CloverThemes"

enum ThemeDownload {
  case indexOnly
  case thumbnail
  case complete
}

enum GitProtocol : String {
  case https = "https"
  case git  = "git"
}

struct ThemeInfo {
  var user: String
  var repo: String
  var optimized : Bool
  
  func archive() -> Data? {
    var dict = [String : String]()
    dict["user"] = self.user
    dict["repo"] = self.repo
    dict["optimized"] = self.optimized ? "1" : "0"
  
    return try? JSONEncoder().encode(dict)
  }
  
  static func unarchive(at path: String) -> ThemeInfo? {
    if let data = try? Data(contentsOf: URL(fileURLWithPath: path)) {
      
      if let dict = try? JSONSerialization.jsonObject(with: data, options: .mutableContainers) as? [String : String] {
        
        let new : ThemeInfo = ThemeInfo(user: dict["user"] ?? "",
                                        repo: dict["repo"] ?? "",
                                        optimized: (dict["optimized"] == "1") ? true : false)
        /*
        print(new.user)
        print(new.repo)
        print(new.optimized)
        */
        return new
      }
    }
    
    return nil
  }
}

let kCloverThemeAttributeKey = "org.cloverTheme.sha"

final class ThemeManager: NSObject, URLSessionDataDelegate {
  private let errorDomain : String = "org.slice.Clover.ThemeManager.Error"
  var statusError : Error? = nil
  var delegate : ThemeManagerVC?
  var user : String
  var repo : String
  var basePath : String
  private var urlBaseStr : String
  var themeManagerIndexDir : String
  
  private var gitInitCount : Int32 = 0

  let userAgent = "Clover"
  
  required init(user: String, repo: String,
                basePath: String,
                indexDir : String,
                delegate: ThemeManagerVC?) {
    self.user = user
    self.repo = repo
    self.basePath = basePath
    self.urlBaseStr = "\(GitProtocol.https.rawValue)://api.github.com/repos/\(user)/\(repo)/git/trees/master?recursive=1"
    self.themeManagerIndexDir = indexDir
    self.delegate = delegate
    if !fm.fileExists(atPath: self.themeManagerIndexDir) {
      try? fm.createDirectory(atPath: self.themeManagerIndexDir,
                              withIntermediateDirectories: true,
                              attributes: nil)
    }
  }

  public func getIndexedThemesForAllRepositories() -> [String] {
    self.statusError = nil
    var themes = [String]()
    let tip = NSHomeDirectory().addPath("Library/Application Support/CloverApp/Themeindex")
    
    if let repos = try? fm.contentsOfDirectory(atPath: tip) {
      for repo in repos {
        let repoThemesDir = tip.addPath(repo).addPath("Themes")
        let repoShaFilePath = tip.addPath(repo).addPath("sha")
        if fm.fileExists(atPath: repoShaFilePath) && fm.fileExists(atPath: repoThemesDir) {
          if let files : [String] = try? fm.contentsOfDirectory(atPath: repoThemesDir) {
            for f in files {
              let theme : String = f.deletingFileExtension
              let ext : String = f.fileExtension
              if ext == "plist" && !themes.contains(theme) {
                themes.append(theme)
              }
            }
          }
        }
      }
    }
    return themes.sorted()
  }
  
  public func getIndexedThemes() -> [String] {
    self.statusError = nil
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
    self.statusError = nil
    var themes : [String] = [String]()
    let themesIndexPath : String = self.themeManagerIndexDir.addPath("Themes")
    
    self.getInfo(urlString: self.urlBaseStr) { (success) in
     
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
  
  private func downloadFile(at url: String, dst: String,
                            completion: @escaping (Bool) -> ()) {
    
    if let validURL : URL = URL(string: self.normalize(url)) {
      let upperDir : String = dst.deletingLastPath
      if !fm.fileExists(atPath: upperDir) {
        do {
          try fm.createDirectory(atPath: upperDir,
                                 withIntermediateDirectories: true,
                                 attributes: nil)
        } catch {
          let errStr = "DF0, \(error.localizedDescription)."
          let se : Error = NSError(domain: self.errorDomain,
                                   code: 1000,
                                   userInfo: [NSLocalizedDescriptionKey: errStr])
          statusError = se
          completion(false)
          return
        }
      }
      
      var request : URLRequest = URLRequest(url: validURL)
      request.httpMethod = "GET"
      request.setValue(self.userAgent, forHTTPHeaderField: "User-Agent")
      
      let config = URLSessionConfiguration.ephemeral
      let session = URLSession(configuration: config)
      
      let task = session.dataTask(with: request, completionHandler: {(d, r, e) in
        if let response = r as? HTTPURLResponse {
          switch response.statusCode {
          case 200:
            break
          default:
            let errStr = "DF1, Error: request for '\(validURL)' response with status code \(response.statusCode) (\(gHTTPInfo(for: response.statusCode)))."
            let se : Error = NSError(domain: self.errorDomain,
                                     code: 1001,
                                     userInfo: [NSLocalizedDescriptionKey: errStr])
            self.statusError = se
            completion(false)
            return
          }
        } else {
          let errStr = "DF2, Error: empty response for '\(validURL)'."
          let se : Error = NSError(domain: self.errorDomain,
                                   code: 1002,
                                   userInfo: [NSLocalizedDescriptionKey: errStr])
          self.statusError = se
          completion(false)
          return
        }
        
        if (e != nil) {
          let errStr = "DF3, \(e!.localizedDescription)."
          let se : Error = NSError(domain: self.errorDomain,
                                   code: 1003,
                                   userInfo: [NSLocalizedDescriptionKey: errStr])
          self.statusError = se
          completion(false)
          return
        }
        guard let data = d else {
          let errStr = "DF4, Error: no datas."
          let se : Error = NSError(domain: self.errorDomain,
                                   code: 1004,
                                   userInfo: [NSLocalizedDescriptionKey: errStr])
          self.statusError = se
          completion(false)
          return
        }
        do {
          try data.write(to: URL(fileURLWithPath: dst))
          completion(true)
        } catch {
          let errStr = "DF5, \(error.localizedDescription)"
          let se : Error = NSError(domain: self.errorDomain,
                                   code: 1005,
                                   userInfo: [NSLocalizedDescriptionKey: errStr])
          self.statusError = se
          completion(false)
        }
      })
      
      task.resume()
    } else {
      let errStr = "DF6, Error: invalid url '\(url)'."
      let se : Error = NSError(domain: self.errorDomain,
                               code: 1006,
                               userInfo: [NSLocalizedDescriptionKey: errStr])
      
      self.statusError = se
      completion(false)
    }
  }

  private func getInfo(urlString: String, completion: @escaping (Bool) -> ()) {
    if let url = URL(string: self.normalize(urlString)) {
      var request : URLRequest = URLRequest(url: url)
      request.httpMethod = "GET"
      request.setValue("application/json; charset=utf-8", forHTTPHeaderField: "Content-Type")
      request.setValue(self.userAgent, forHTTPHeaderField: "User-Agent")
      let config = URLSessionConfiguration.ephemeral
      let session = URLSession(configuration: config)
      
      let task = session.dataTask(with: request, completionHandler: {(d, r, e) in
        if let reponse = r as? HTTPURLResponse {
          if reponse.statusCode != 200 {
            let errStr = "GI0, Error: request for '\(url)' reponse with status code \(reponse.statusCode) (\(gHTTPInfo(for: reponse.statusCode)))."
            let se : Error = NSError(domain: self.errorDomain,
                                     code: 2000,
                                     userInfo: [NSLocalizedDescriptionKey: errStr])
            self.statusError = se
            completion(false)
            return
          }
        }
        
        if (e != nil) {
          let errStr = "GI1, \(e!.localizedDescription)"
          let se : Error = NSError(domain: self.errorDomain,
                                   code: 2001,
                                   userInfo: [NSLocalizedDescriptionKey: errStr])
          
          self.statusError = se
          completion(false)
          return
        }
        
        guard let data = d else {
          let errStr = "GI2, no data"
          let se : Error = NSError(domain: self.errorDomain,
                                   code: 2002,
                                   userInfo: [NSLocalizedDescriptionKey: errStr])
          
          self.statusError = se
          completion(false)
          return
        }
        
        guard let utf8 = String(decoding: data, as: UTF8.self).data(using: .utf8) else {
          let errStr = "GI3, data is not utf8."
          let se : Error = NSError(domain: self.errorDomain,
                                   code: 2003,
                                   userInfo: [NSLocalizedDescriptionKey: errStr])
          
          self.statusError = se
          completion(false)
          return
        }
        
        do {
          let json = try JSONSerialization.jsonObject(with: utf8, options: .mutableContainers)
       
          if let jdict = json as? [String: Any] {
            guard let truncated = jdict["truncated"] as? Bool else {
              let errStr = "GI4, Error: 'truncated' key not found"
              let se : Error = NSError(domain: self.errorDomain,
                                       code: 2004,
                                       userInfo: [NSLocalizedDescriptionKey: errStr])
              
              self.statusError = se
              completion(false)
              return
            }
            
            if truncated == true {
              let errStr = "GI5, Error: json has truncated list."
              let se : Error = NSError(domain: self.errorDomain,
                                       code: 2005,
                                       userInfo: [NSLocalizedDescriptionKey: errStr])
              
              self.statusError = se
              completion(false)
              return
            }
            
            guard let sha = jdict["sha"] as? String else {
              let errStr = "GI6, Error: 'sha' key not found"
              let se : Error = NSError(domain: self.errorDomain,
                                       code: 2006,
                                       userInfo: [NSLocalizedDescriptionKey: errStr])
              
              self.statusError = se
              completion(false)
              return
            }
            
            if sha == self.getSha() {
              // we're done because we already have all the files needed
              completion(false)
              return
            }
            
            guard let tree = jdict["tree"] as? [[String: Any]] else {
              let errStr = "GI7, Error: 'tree' key not found, or not an array."
              let se : Error = NSError(domain: self.errorDomain,
                                       code: 2007,
                                       userInfo: [NSLocalizedDescriptionKey: errStr])
              
              self.statusError = se
              completion(false)
              return
            }
            
            let shaPath : String = "\(self.themeManagerIndexDir)/\(sha)"
            
            do {
              if !fm.fileExists(atPath: shaPath) {
                try fm.createDirectory(atPath: shaPath, withIntermediateDirectories: true, attributes: nil)
              }
              
            } catch {
              let errStr = "GI8, Error: can't write sha commit."
              let se : Error = NSError(domain: self.errorDomain,
                                       code: 2008,
                                       userInfo: [NSLocalizedDescriptionKey: errStr])
              
              self.statusError = se
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
                let errStr = "GI9, Error: 'type' key not found."
                let se : Error = NSError(domain: self.errorDomain,
                                         code: 2009,
                                         userInfo: [NSLocalizedDescriptionKey: errStr])
                
                self.statusError = se
                completion(false)
                break
              }
            
              if let path = obj["path"] as? String {
                if path.componentsPath.count == 1 && type != "tree" {
                  // skip every things is not a directory in the root of the repository (like README.md)
                  continue
                }
                
                guard let fileSha = obj["sha"] as? String else {
                  let errStr = "GI16, Error: 'sha' key not found for \(path)."
                  let se : Error = NSError(domain: self.errorDomain,
                                           code: 2016,
                                           userInfo: [NSLocalizedDescriptionKey: errStr])
                  
                  self.statusError = se
                  completion(false)
                  break
                }
                
                // skip every hidden files like .gitignore, .DS_Store, etc.
                if !path.hasPrefix(".") && type == "blob" {
                  let themeName : String = path.components(separatedBy: "/")[0]
                  let plistPath : String = "\(self.themeManagerIndexDir)/\(sha)/\(themeName).plist"
                  let theme : NSMutableDictionary = NSMutableDictionary(contentsOfFile: plistPath) ?? NSMutableDictionary()
                  if !(theme.allKeys as! [String]).contains(path) {
                    theme.setObject(fileSha, forKey: path as NSString)
                  }
                  
                  
                  if !theme.write(toFile: plistPath, atomically: false) {
                    let errStr = "GI10, Error: can't write \(plistPath)"
                    let se : Error = NSError(domain: self.errorDomain,
                                             code: 2010,
                                             userInfo: [NSLocalizedDescriptionKey: errStr])
                    
                    self.statusError = se
                    completion(false)
                    break
                  }
                }
              } else {
                let errStr = "GI11, Error: 'path' key not found."
                let se : Error = NSError(domain: self.errorDomain,
                                         code: 2011,
                                         userInfo: [NSLocalizedDescriptionKey: errStr])
                
                self.statusError = se
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
              let errStr = "GI12, \(error.localizedDescription)"
              let se : Error = NSError(domain: self.errorDomain,
                                       code: 2012,
                                       userInfo: [NSLocalizedDescriptionKey: errStr])
              
              self.statusError = se
              completion(false)
              return
            }
            
            //remove old themes (old sha)
          } else {
            let errStr = "GI13, json is not a dictionary (API change?)."
            let se : Error = NSError(domain: self.errorDomain,
                                     code: 2013,
                                     userInfo: [NSLocalizedDescriptionKey: errStr])
            
            self.statusError = se
            completion(false)
          }
         
        } catch {
          let errStr = "GI14, \(error.localizedDescription)"
          let se : Error = NSError(domain: self.errorDomain,
                                   code: 2014,
                                   userInfo: [NSLocalizedDescriptionKey: errStr])
          
          self.statusError = se
          completion(false)
        }
      })
      
      task.resume()
    } else {
      let errStr = "GI15, \(urlString) is invalid."
      let se : Error = NSError(domain: self.errorDomain,
                               code: 2015,
                               userInfo: [NSLocalizedDescriptionKey: errStr])
      
      self.statusError = se
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
  
  private func thumbnailExist(at path: String) -> Bool {
    if fm.fileExists(atPath: path.addPath("theme.svg")) {
      return true
    } else {
      if fm.fileExists(atPath: path.addPath("theme.plist")) &&
        fm.fileExists(atPath: path.addPath("screenshot.png")){
        return true
      }
    }
    return false
  }
  
  /// Return the path for a given theme, if the download succeded
  public func download(theme: String, down: ThemeDownload, completion: @escaping (String?) -> ()) {
    let advice = "Try to refresh the list by pressing the Refresh button below."
    self.statusError = nil
    if let sha = self.getSha() {
      let shaPath : String = self.basePath.addPath(sha)
      let themeDest : String = (down == .complete)
        ? self.themeManagerIndexDir.addPath("Downloads").addPath(theme)
        :  shaPath.addPath(theme)
      if (down != .complete) && self.thumbnailExist(at: themeDest) {
        completion(themeDest)
      } else {
        if !fm.fileExists(atPath: themeDest) {
          do {
            try fm.createDirectory(atPath: themeDest,
                                   withIntermediateDirectories: true,
                                   attributes: nil)
          } catch {
            let desc = "Unexpected error: '\(error)'"
            let e = NSError(domain: "org.slice.Clover.Download.Error",
                            code: 3000,
                            userInfo :[NSLocalizedDescriptionKey : desc])
            
            self.statusError = e
            completion(nil)
            return
          }
        }
        
        let plistPath : String = "\(themeManagerIndexDir)/Themes/\(theme).plist"
        let themePlist = NSDictionary(contentsOfFile: plistPath)
        if let files : [String] = themePlist?.allKeys as? [String] {
          let fc : Int = files.count
          if fc > 0 {
            var succeded : Bool = true
            let dispatchGroup = DispatchGroup()
            for i in 0..<fc {
              let file : String = files[i]
              // build the url
              let furl : String = "\(GitProtocol.https.rawValue)://raw.githubusercontent.com/\(self.user)/\(self.repo)/master/\(file)"
              
              if down == .thumbnail {
                if file != theme.addPath("screenshot.png")
                  && file != theme.addPath("theme.svg")
                  && file != theme.addPath("theme.plist") {
                  continue
                }
              }
              
              let filedest : String = (down == .complete)
                ? themeDest.deletingLastPath.addPath(file)
                : shaPath.addPath(file)
              
              if !fm.fileExists(atPath: filedest) {
                dispatchGroup.enter()
                self.downloadFile(at: self.normalize(furl), dst: filedest) { (success) in
                  succeded = success
                  dispatchGroup.leave()
                }
                if !succeded {
                  break
                }
              }
            }
            
            dispatchGroup.notify(queue: DispatchQueue.main, execute: {
              if succeded {
                completion(themeDest)
              } else {
                if self.statusError == nil { // downloadFile() generate a specific error
                  let desc = "Unknown error downloading '\(theme)' theme."
                  let e = NSError(domain: "org.slice.Clover.Download.Error",
                                  code: 3001,
                                  userInfo :[NSLocalizedDescriptionKey : desc])
                  
                  self.statusError = e
                }
                completion(nil)
              }
            })
          } else {
            let desc = "'\(theme)' index file contains no file to download.\n\n\(advice)"
            let e = NSError(domain: "org.slice.Clover.Download.Error",
                            code: 3002,
                            userInfo :[NSLocalizedDescriptionKey : desc])
            
            self.statusError = e
            completion(nil)
          }
        } else {
          let desc = "Unable to load '\(theme)' index file (not found or unreadable).\n\n\(advice)"
          let e = NSError(domain: "org.slice.Clover.Download.Error",
                          code: 3003,
                          userInfo :[NSLocalizedDescriptionKey : desc])
          
          self.statusError = e
          completion(nil)
        }
      }
    } else {
      let desc = "sha1 directory not found.\n\n\(advice)"
      let e = NSError(domain: "org.slice.Clover.Download.Error",
                      code: 3004,
                      userInfo :[NSLocalizedDescriptionKey : desc])
      
      self.statusError = e
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
  
  public func signTheme(at path: String) { // unused function
    if let sha : String = self.getSha() {
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
  
  /// Check if a theme exist (in the current repository
  public func exist(theme: String) -> Bool {
    return fm.fileExists(atPath: "\(self.themeManagerIndexDir)/Themes/\(theme).plist")
  }
  
  /// Check if a theme at the given path is up to date in the current repository. True if doesn't exists.
  public func isThemeUpToDate(at path : String) -> Bool {
    let theme = path.lastPath
    if !self.exist(theme: theme) {
      // if theme doesn't exist in the repository is up to date since is not part of it!
      return true
    }
   
    /*
     if the repo is not the current one return true
     */
    
    var isOptimized = false
    
    if let info = ThemeInfo.unarchive(at: path.addPath(kThemeInfoFile)) {
      if info.user != self.user || info.repo != self.repo {
        return true
      }
      isOptimized = info.optimized
    }
    
    let plistPath = "\(self.themeManagerIndexDir)/Themes/\(theme).plist"
    guard var plist = NSMutableDictionary(contentsOfFile: plistPath) as? [String : String] else {
      // We cannot load our generated file? :-(
      print("Error: Theme Manager can't load \(plistPath)")
      return true
    }
    
    let enumerator = fm.enumerator(atPath: path)
    
    while let file = enumerator?.nextObject() as? String {
      let fp = path.addPath(file)
      var isDir : ObjCBool = false
      fm.fileExists(atPath: fp, isDirectory: &isDir)
      
      // don't check hidden files, like .DS_Store ;-)
      if !file.lastPath.hasPrefix(".") {
        /*
         Check only files.
         If extra directories exists inside the user theme it's not our business...
         */
        
        if !isDir.boolValue {
          // ..it is if a file doesn't exist
          let key = theme.addPath(file)
          
          if let githubSha = plist[key] {
            /*
             if the theme is optimized check only file existence.
             otherwise compare the sha1
             */
            if !isOptimized {
              // ok compare the sha1
              if let fileData = try? Data(contentsOf: URL(fileURLWithPath: fp)) {
                var gitdata = Data()
                let encoding : String.Encoding = .utf8
                gitdata.append("blob ".data(using: encoding)!)
                gitdata.append("\(fileData.count)".data(using: encoding)!)
                gitdata.append(0x00)
                gitdata.append(fileData)
                
                let gitsha1 = gitdata.sha1
                if gitsha1 != githubSha {
                  // sha doesn't match, this is a different file
                  return false
                }
              } else {
                print("Error: Theme Manager can't load \(fp)")
                return false
              }
            }
            
            plist[key] = nil // no longer needed
          } else {
            // file doesn't exist in the theme at repository
            return false
          }
        }
      }
    }
    
    /*
     if We are here is because sha1 has been compared successfully
     for each file on the installed theme.
     If plist var no longer contains keys, it also means that all
     the files are existing on the installed theme.
     Otherwise fail!
     */
    return plist.keys.count == 0
  }
  
  private func getIconSize(configPlistPath: String) -> Int {
    let config = NSDictionary(contentsOfFile: configPlistPath) as? [String : Any]
    
    let minScreenWidth : Int = 1280
    var screenWidth : Int = minScreenWidth
    let ScreenResolution = ((config?["GUI"] as? [String : Any])?["ScreenResolution"] as? String) ?? ""
    
    if ScreenResolution.count > 0 {
      screenWidth = Int(ScreenResolution.components(separatedBy: "x")[0]) ?? 0
    } else {
      if let mainScreenFrame = NSScreen.main?.frame {
        screenWidth = Int(mainScreenFrame.size.width)
      }
    }
    
    if screenWidth < minScreenWidth {
      screenWidth = minScreenWidth // default
    }
    
    // determine the icon size
    var iconSize : Int = 128 // default
    switch screenWidth {
    case 0...2048:
      iconSize = 128
    case 2049...2560:
      iconSize = 192
    case 2561...3840:
      iconSize = 256
    case 3841...7680:
      iconSize = 400
    case 7681...12000:
      iconSize = 512
    default:
      break
    }
    
    return iconSize
  }
  
  public func optimizeTheme(at path: String, completion: @escaping (Error?) -> ()) {
    DispatchQueue.global(priority: .background).async(execute: { () -> Void in
      let plist = NSDictionary(contentsOfFile: path.addPath("theme.plist")) as? [String : Any]
      let theme = plist?["Theme"] as? [String : Any]
      let Selection = plist?["Selection"] as? [String : Any]
      
      //logo 128x128 pixels  Theme->Banner
      let logo : String = (theme?["Banner"] as? String) ?? "logo.png"
      
      // selection_big 64x64 pixels   Theme->Selection->Big
      let Selection_big : String = (Selection?["Big"] as? String) ?? "Selection_big.png"
      
      // selection_small 144x144 pixels   Theme->Selection->Small
      let Selection_small : String = (Selection?["Small"] as? String) ?? "Selection_small.png"
      
      var images : [String] = [String]()
      let enumerator = fm.enumerator(atPath: path)
      
      if var info = ThemeInfo.unarchive(at: path.addPath(kThemeInfoFile)) {
        info.optimized = true
        // write back
        try? info.archive()?.write(to: URL(fileURLWithPath: path.addPath(kThemeInfoFile)))
      }
      
      var minIconSizeWidth : CGFloat? = nil
      var iconsNeedsResize : Bool = false
      let cloverDir = path.deletingLastPath.deletingLastPath
      let suggestedSize = self.getIconSize(configPlistPath: cloverDir.addPath("config.plist"))
      
      while let file = enumerator?.nextObject() as? String {
        if file.fileExtension == "png" || file.fileExtension == "icns" {
          images.append(file)
          // load icons to see if they have the same
          if file.hasPrefix("icons/os_") || file.hasPrefix("icons/vol_") {
            if let image = NSImage(byReferencingFile: path.addPath(file)) {
              // ensure width and height are equal
              if image.size.width != image.size.height {
                iconsNeedsResize = true
              }
              if minIconSizeWidth != nil {
                // size is same as previous in previous icon?
                if minIconSizeWidth != image.size.width {
                  iconsNeedsResize = true
                }
                
                // is this image smaller?
                if image.size.width < minIconSizeWidth! {
                  minIconSizeWidth = image.size.width
                }
              } else {
                minIconSizeWidth = image.size.width
              }
            }
          }
        }
      }
      
      if iconsNeedsResize {
        // is minIconSizeWidth resonable fro suggestedSize?
        if minIconSizeWidth != nil && minIconSizeWidth! <= CGFloat(suggestedSize)  {
          minIconSizeWidth = CGFloat(suggestedSize)
        }
      }
      
      for file in images {
        let fullPath = path.addPath(file)
        if var image = NSImage(byReferencingFile: fullPath) {
          do {
            let size = image.size
            let fileName = fullPath.lastPath
            if file.hasPrefix("icons/") {
              if (fileName.hasPrefix("os_") || fileName.hasPrefix("vol_")) {
                if iconsNeedsResize {
                  image = image.resize(to: NSMakeSize(minIconSizeWidth!, minIconSizeWidth!))
                }
              } else if (fileName.hasPrefix("func_") ||
                fileName.hasPrefix("tool_") ||
                fileName.hasPrefix("pointer.")) { // 32x32 pixels
                if size.width != 32 || size.height != 32 {
                  image = image.resize(to: NSMakeSize(32, 32))
                }
              }
            } else if file.hasPrefix("scrollbar/") {
              if fileName.hasPrefix("bar_end.") || fileName.hasPrefix("bar_start.") {
                image = image.resize(to: NSMakeSize(16, 5))
              } else if fileName.hasPrefix("bar_fill.") || fileName.hasPrefix("scroll_fill.") {
                image = image.resize(to: NSMakeSize(16, 1))
              } else if fileName.hasPrefix("down_button.") || fileName.hasPrefix("up_button.") {
                image = image.resize(to: NSMakeSize(16, 20))
              } else if fileName.hasPrefix("scroll_end.") || fileName.hasPrefix("scroll_start.")  {
                image = image.resize(to: NSMakeSize(16, 7))
              }
            } else {
              if file == logo { // logo 128x128 pixels
                if size.width != 128 || size.height != 128 {
                  image = image.resize(to: NSMakeSize(128, 128))
                }
              } else if file == Selection_big { // selection_big 144x144 pixels
                if size.width != 144 || size.height != 144 {
                  image = image.resize(to: NSMakeSize(144, 144))
                }
              } else if file == Selection_small { // selection_small 64x64 pixels
                if size.width != 64 || size.height != 64 {
                  image = image.resize(to: NSMakeSize(64, 64))
                }
              } else if (fileName.hasPrefix("radio_button.") ||
                fileName.hasPrefix("radio_button_selected.") ||
                fileName.hasPrefix("checkbox.") ||
                fileName.hasPrefix("checkbox_checked.")) { // 15x15 pixels
                if size.width != 15 || size.height != 15 {
                  image = image.resize(to: NSMakeSize(15, 15))
                }
              }
            }
            
            if fullPath.fileExtension == "icns" {
              try fm.removeItem(atPath: fullPath)
            }
            
            // everythings has now png extensions
            // optimize
            var success = false
            if let tr = image.tiffRepresentation {
              if let bitmapImage = NSBitmapImageRep(data: tr) {
                if let data = bitmapImage.representation(using: .png, properties: [:]) {
                  var err : NSError?  = nil
                  let optimizedData = ThemeImage(data: data, error: &err, atPath: fullPath)?.pngData
                  try optimizedData?.write(to: URL(fileURLWithPath: "\(fullPath.deletingFileExtension).png"))
                  if (err != nil) {
                    completion(err)
                    break
                  }
                  success = true
                }
              }
            }
            
            if !success {
              let desc = "Unable to optimize \(fullPath) data."
              let e = NSError(domain: "org.slice.Clover.NSImage.Error",
                              code: 1001,
                              userInfo :[NSLocalizedDescriptionKey : desc])
              completion(e)
              break
            }
          } catch {
            completion(error)
            break
          }
        } else {
          let desc = "Unable to load \(fullPath)."
          let e = NSError(domain: "org.slice.Clover.NSImage.Error",
                          code: 1000,
                          userInfo :[NSLocalizedDescriptionKey : desc])
          completion(e)
          break
        }
        
        
        if file == images.last {
          completion(nil)
        }
      }
    })
  }
}
