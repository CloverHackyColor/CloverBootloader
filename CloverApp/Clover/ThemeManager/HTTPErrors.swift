//
//  HTTPErrors.swift
//  Clover
//
//  Created by vector sigma on 27/03/2020.
//  Copyright © 2020 CloverHackyColor. All rights reserved.
//

import Foundation

func gHTTPInfo(for statusCode: Int) -> String {
  var info = "Unknown"
  switch statusCode {
  case 100: info = "Continue"
  case 101: info = "Switching Protocols"
  case 102: info = "Processing"
  case 200: info = "OK"
  case 201: info = "Created"
  case 202: info = "Accepted"
  case 203: info = "Non-authoritative Information"
  case 204: info = "No Content"
  case 205: info = "Reset Content"
  case 206: info = "Partial Content"
  case 207: info = "Multi-Status"
  case 208: info = "Already Reported"
  case 226: info = "IM Used"
  case 300: info = "Multiple Choices"
  case 301: info = "Moved Permanently"
  case 302: info = "Found"
  case 303: info = "See Other"
  case 304: info = "Not Modified"
  case 305: info = "Use Proxy"
  case 307: info = "Temporary Redirect"
  case 308: info = "Permanent Redirect"
  case 400: info = "Bad Request"
  case 401: info = "Unauthorized"
  case 402: info = "Payment Required"
  case 403: info = "Forbidden"
  case 404: info = "Not Found"
  case 405: info = "Method Not Allowed"
  case 406: info = "Not Acceptable"
  case 407: info = "Proxy Authentication Required"
  case 408: info = "Request Timeout"
  case 409: info = "Conflict"
  case 410: info = "Gone"
  case 411: info = "Length Required"
  case 412: info = "Precondition Failed"
  case 413: info = "Payload Too Large"
  case 414: info = "Request-URI Too Long"
  case 415: info = "Unsupported Media Type"
  case 416: info = "Requested Range Not Satisfiable"
  case 417: info = "Expectation Failed"
  case 418: info = "I'm a teapot"
  case 421: info = "Misdirected Request"
  case 422: info = "Unprocessable Entity"
  case 423: info = "Locked"
  case 424: info = "Failed Dependency"
  case 426: info = "Upgrade Required"
  case 428: info = "Precondition Required"
  case 429: info = "Too Many Requests"
  case 431: info = "Request Header Fields Too Large"
  case 444: info = "Connection Closed Without Response"
  case 451: info = "Unavailable For Legal Reasons"
  case 499: info = "Client Closed Request"
  case 500: info = "Internal Server Error"
  case 501: info = "Not Implemented"
  case 502: info = "Bad Gateway"
  case 503: info = "Service Unavailable"
  case 504: info = "Gateway Timeout"
  case 505: info = "HTTP Version Not Supported"
  case 506: info = "Variant Also Negotiates"
  case 507: info = "Insufficient Storage"
  case 508: info = "Loop Detected"
  case 510: info = "Not Extended"
  case 511: info = "Network Authentication Required"
  case 599: info = "Network Connect Timeout Error"
  default:
    break
  }
  
  return info
}
