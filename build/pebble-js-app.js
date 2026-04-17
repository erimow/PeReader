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
	const MAX_PAGE_LENGTH = 90; // SAFE for Pebble
	
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
	}
	
	// ===== PAGE SPLITTING =====
	function splitIntoPages(text, maxLength = MAX_PAGE_LENGTH) {
	  const paragraphs = text.split(/\n+/);
	  const pages = [];
	
	  paragraphs.forEach(p => {
	    if (!p.trim()) return; // skip empty paragraphs
	
	    let words = p.split(/\s+/);
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
	      <h2>PeReader Upload</h2>
	
	      <input type="file" id="fileInput" accept=".txt" />
	      <br><br>
	      <button id="uploadBtn">Upload</button>
	
	      <script>
	        let processedPages = [];
	
	        function splitIntoPages(text, maxLength = 90) {
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
	            processedPages = splitIntoPages(text);
	
	            alert("Ready! Pages: " + processedPages.length);
	          };
	
	          reader.readAsText(file, "UTF-8");
	        });
	
	        document.getElementById('uploadBtn').addEventListener('click', function() {
	          if (!processedPages.length) {
	            alert("Select a file first!");
	            return;
	          }
	
	          document.location = "pebblejs://close#" + encodeURIComponent(JSON.stringify({
	            pages: processedPages
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