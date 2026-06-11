/******/ (function(modules) { // webpackBootstrap
/******/ 	// The module cache
/******/ 	var installedModules = {};
/******/
/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {
/******/
/******/ 		// Check if module is in cache
/******/ 		if(installedModules[moduleId])
/******/ 			return installedModules[moduleId].exports;
/******/
/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = installedModules[moduleId] = {
/******/ 			exports: {},
/******/ 			id: moduleId,
/******/ 			loaded: false
/******/ 		};
/******/
/******/ 		// Execute the module function
/******/ 		modules[moduleId].call(module.exports, module, module.exports, __webpack_require__);
/******/
/******/ 		// Flag the module as loaded
/******/ 		module.loaded = true;
/******/
/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}
/******/
/******/
/******/ 	// expose the modules object (__webpack_modules__)
/******/ 	__webpack_require__.m = modules;
/******/
/******/ 	// expose the module cache
/******/ 	__webpack_require__.c = installedModules;
/******/
/******/ 	// __webpack_public_path__
/******/ 	__webpack_require__.p = "";
/******/
/******/ 	// Load entry module and return exports
/******/ 	return __webpack_require__(0);
/******/ })
/************************************************************************/
/******/ ([
/* 0 */
/***/ (function(module, exports, __webpack_require__) {

	__webpack_require__(1);
	module.exports = __webpack_require__(2);


/***/ }),
/* 1 */
/***/ (function(module, exports) {

	/**
	 * Copyright 2024 Google LLC
	 *
	 * Licensed under the Apache License, Version 2.0 (the "License");
	 * you may not use this file except in compliance with the License.
	 * You may obtain a copy of the License at
	 *
	 *     http://www.apache.org/licenses/LICENSE-2.0
	 *
	 * Unless required by applicable law or agreed to in writing, software
	 * distributed under the License is distributed on an "AS IS" BASIS,
	 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	 * See the License for the specific language governing permissions and
	 * limitations under the License.
	 */
	
	(function(p) {
	  if (!p === undefined) {
	    console.error('Pebble object not found!?');
	    return;
	  }
	
	  // Aliases:
	  p.on = p.addEventListener;
	  p.off = p.removeEventListener;
	
	  // For Android (WebView-based) pkjs, print stacktrace for uncaught errors:
	  if (typeof window !== 'undefined' && window.addEventListener) {
	    window.addEventListener('error', function(event) {
	      if (event.error && event.error.stack) {
	        console.error('' + event.error + '\n' + event.error.stack);
	      }
	    });
	  }
	
	})(Pebble);


/***/ }),
/* 2 */
/***/ (function(module, exports) {

	// ===== CONFIG =====
	let MAX_PAGE_LENGTH = 90; // SAFE for Pebble
	
	// ===== STATE =====
	let pages = [];
	let currentPage = 0;
	
	// ===== LOAD SAVED BOOK =====
	function loadBook() {
	  try {
	    pages = JSON.parse(localStorage.getItem("book_pages") || "[]");
	    } catch (e) {
	    pages = [];
	  }
	    try {MAX_PAGE_LENGTH = JSON.parse(localStorage.getItem("max_length"));}
	    catch (e){
	      MAX_PAGE_LENGTH=90;
	    }
	}
	
	// ===== PAGE SPLITTING =====
	function splitIntoPages(text, maxLength = MAX_PAGE_LENGTH) {
	  const paragraphs = text.split(/\\n+/);
	  const pages = [];
	
	  paragraphs.forEach(p => {
	    if (!p.trim()) return; // skip empty paragraphs
	
	    let words = p.split(/\\s+/);
	    let current = "";
	
	    words.forEach(word => {
	      if ((current + word).length > maxLength) {
	        pages.push(current.trim());
	        current = "";
	      }
	      current += word + " ";
	    });
	
	    if (current.trim()) {
	      pages.push(current.trim());
	    }
	  });
	
	  return pages;
	}
	
	// ===== SEND PAGE TO WATCH =====
	function sendPage() {
	  if (!pages.length) {
	    Pebble.sendAppMessage({
	      PAGE_TEXT: "No book loaded",
	      TOTAL_PAGES: 0
	    });
	    return;
	  }
	
	  let pageText = pages[currentPage];
	
	  if (!pageText || !pageText.trim()) {
	    pageText = " ";
	  }
	
	  // HARD CLAMP (prevents corruption)
	  if (pageText.length > MAX_PAGE_LENGTH) {
	    pageText = pageText.substring(0, MAX_PAGE_LENGTH);
	  }
	
	  Pebble.sendAppMessage({
	    PAGE_TEXT: pageText,
	    TOTAL_PAGES: pages.length
	  });
	}
	
	// ===== HANDLE REQUEST FROM WATCH =====
	Pebble.addEventListener("appmessage", function(e) {
	  if (e.payload.PAGE_REQUEST !== undefined) {
	    currentPage = e.payload.PAGE_REQUEST;
	
	    if (currentPage < 0) currentPage = 0;
	    if (currentPage >= pages.length) currentPage = pages.length - 1;
	
	    sendPage();
	  }
	});
	
	// ===== CONFIG PAGE =====
	Pebble.addEventListener("showConfiguration", function() {
	  const html = `
	    <html>
	    <body style="font-family: sans-serif; padding: 20px;">
	      <h2 style="color: white;">PeReader Upload</h2>
	
	      <label for="fileupload" style="color: white;">Upload TXT file below</label>
	      <br>
	      <input type="file" id="fileInput" accept=".txt" />
	      <br><br>
	      <label for="maxlength" style="color: white;">Set the maximum amount of characters per page.\n (90 seems to work well for smaller screens ie. time/steel)\n</label>
	      <br>
	      <input type="number" id="maxlength" name="Max Length" min="10" max="500" value="90"/>
	      <br><br>
	      <label for="uploadbutton" style="color: white;">Then Upload\n</label>
	      <br>
	      <button id="uploadBtn">Upload</button>
	      <br><br>
	      <label id="errorlabel" for="errorlabel" style="color: red;"></label>
	
	      <script>
	        let processedPages = [];
	        let max_length = 90;
	
	        function splitIntoPages(text, maxLength = document.getElementById("maxlength").value) {
	          max_length = maxLength;
	          const paragraphs = text.split(/\\n+/);
	          const pages = [];
	
	          paragraphs.forEach(p => {
	            let words = p.split(/\\s+/);
	            let current = "";
	
	            words.forEach(word => {
	              if ((current + word).length > maxLength) {
	                pages.push(current.trim());
	                current = "";
	              }
	              current += word + " ";
	            });
	
	            if (current) pages.push(current.trim());
	          });
	
	          return pages;
	        }
	
	        document.getElementById('fileInput').addEventListener('change', function(e) {
	          const file = e.target.files[0];
	          const reader = new FileReader();
	
	          reader.onload = function(e) {
	            const text = e.target.result;
	
	            // text = text
	            //   .replace(/\\r\\n/g, "\\n")
	            //   .replace(/\\u2028|\\u2029/g, "\\n")
	            //   .replace(/\\u0000/g, "");
	            
	            processedPages = splitIntoPages(text);
	
	            alert("Ready! Pages: " + processedPages.length);
	          };
	
	          reader.readAsText(file, "UTF-8");
	        });
	
	        document.getElementById('uploadBtn').addEventListener('click', function() {
	          if (!processedPages.length) {
	            alert("Select a file first!");
	            document.getElementById('errorlabel').innerHTML = "Select a file first!";
	            return;
	          }
	           
	          //const payload = JSON.stringify({ pages: processedPages });
	          //document.getElementById('errorlabel').innerHTML = "JSON Success, length = " + payload.length + "encode length = " + encodeURIComponent(payload).length;
	
	
	          document.location = "pebblejs://close#" + encodeURIComponent(JSON.stringify({
	            pages: processedPages,
	            maxlength: max_length
	          }));
	        });
	      </script>
	    </body>
	    </html>
	  `;
	
	  Pebble.openURL("data:text/html," + encodeURIComponent(html));
	});
	
	// ===== RECEIVE DATA FROM CONFIG =====
	Pebble.addEventListener("webviewclosed", function(e) {
	  if (!e.response) return;
	
	  let data = JSON.parse(decodeURIComponent(e.response));
	
	  if (data.maxlength){
	    localStorage.setItem("max_length", data.maxlength);
	    MAX_PAGE_LENGTH = data.maxlength;
	  }
	  if (data.pages) {
	    localStorage.setItem("book_pages", JSON.stringify(data.pages));
	    pages = data.pages;
	    currentPage = 0;
	
	    console.log("Saved book with", pages.length, "pages");
	
	    // Auto refresh watch
	    sendPage();
	  }
	});
	
	// ===== INIT =====
	Pebble.addEventListener("ready", function() {
	  console.log("PebbleKit JS ready!");
	  loadBook();
	});

/***/ })
/******/ ]);
//# sourceMappingURL=pebble-js-app.js.map