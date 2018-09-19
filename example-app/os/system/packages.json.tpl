"default/About": {
	"className": "ApplicationAbout",
	"name": "About OS.js",
	"description": "About OS.js",
	"names": {
		"bg_BG": " За OS.js",
		"de_DE": "Über OS.js",
		"fr_FR": "À propos d'OS.js",
		"it_IT": "Informazioni su OS.js",
		"ko_KR": "OS.js에 대하여",
		"nl_NL": "Over OS.js",
		"no_NO": "Om OS.js",
		"pl_PL": "o OS.js",
		"ru_RU": "Об OS.js",
		"sk_SK": "o OS.js",
		"tr_TR": "hakkında OS.js",
		"vi_VN": "Thông tin về OS.js"
	},
	"descriptions": {
		"bg_BG": "За OS.js",
		"de_DE": "Über OS.js",
		"fr_FR": "À propos d'OS.js",
		"it_IT": "Informazioni su OS.js",
		"ko_KR": "OS.js에 대하여",
		"nl_NL": "Over OS.js",
		"no_NO": "Om OS.js",
		"pl_PL": "o OS.js",
		"ru_RU": "Об OS.js",
		"sk_SK": "o OS.js",
		"tr_TR": "hakkında OS.js",
		"vi_VN": "Thông tin về OS.js"
	},
	"singular": true,
	"category": "system",
	"icon": "apps/help-browser.png",
	"preload": [{
		"type": "javascript",
		"src": "combined.js"
	}, {
		"type": "stylesheet",
		"src": "combined.css"
	}, {
		"src": "scheme.html",
		"type": "scheme"
	}],
	"type": "application",
	"path": "default/About",
	"build": {},
	"repo": "default"
},
"default/AceEditor": {
	"className": "ApplicationAceEditor",
	"name": "Source Editor",
	"icon": "apps/accessories-text-editor.png",
	"category": "development",
	"mime": [
		"^text",
		"inode\\/x\\-empty",
		"application\\/x\\-empty",
		"application\\/x\\-python",
		"application\\/x\\-php",
		"application\\/javascript"
	],
	"build": {
		"copy": [
			"metadata.json",
			"scheme.html",
			"main.css",
			"main.js",
			"vendor/ace"
		]
	},
	"preload": [{
		"type": "javascript",
		"src": "combined.js"
	}, {
		"type": "stylesheet",
		"src": "combined.css"
	}, {
		"src": "scheme.html",
		"type": "scheme"
	}],
	"type": "application",
	"path": "default/AceEditor",
	"repo": "default"
},
"default/Archiver": {
	"className": "ApplicationArchiver",
	"name": "Archiver",
	"mime": [
		"application/zip"
	],
	"icon": "apps/system-software-install.png",
	"category": "utilities",
	"compability": [
		"file",
		"blob"
	],
	"preload": [{
		"type": "javascript",
		"src": "combined.js"
	}, {
		"type": "stylesheet",
		"src": "combined.css"
	}, {
		"src": "scheme.html",
		"type": "scheme"
	}],
	"type": "application",
	"path": "default/Archiver",
	"build": {},
	"repo": "default"
},
"default/Calculator": {
	"className": "ApplicationCalculator",
	"name": "Calculator",
	"names": {
		"bg_Bg": "Клакулатор",
		"fr_FR": "Calculatrice",
		"it_IT": "Calcolatrice",
		"ko_KR": "계산기",
		"nl_NL": "Rekenmachine",
		"no_NO": "Kalkulator",
		"pl_PL": "Kalkulator",
		"ru_RU": "Калькулятор",
		"sk_SK": "Kalkulačka",
		"tr_TR": "Hesap Makinesi",
		"vi_VN": "Máy tính"
	},
	"icon": "apps/calc.png",
	"category": "office",
	"preload": [{
		"type": "javascript",
		"src": "combined.js"
	}, {
		"type": "stylesheet",
		"src": "combined.css"
	}, {
		"src": "scheme.html",
		"type": "scheme"
	}],
	"type": "application",
	"path": "default/Calculator",
	"build": {},
	"repo": "default"
},
"default/CoreWM": {
	"className": "CoreWM",
	"name": "OS.js Window Manager",
	"names": {
		"bg_BG": "Мениджър на прозорци на OS.js",
		"de_DE": "OS.js Fenster-Manager",
		"es_ES": "OS.js Window Manager",
		"fr_FR": "Gestionnaire de fenêtre OS.js",
		"it_IT": "OS.js Gestore Finestre",
		"ko_KR": "OS.js 윈도우 관리자",
		"nl_NL": "OS.js venster beheer",
		"no_NO": "OS.js Vinduhåndterer",
		"pl_PL": "Menedżer Okien OS.js",
		"ru_RU": "OS.js Оконный менеджер",
		"sk_SK": "Správca Okien OS.js",
		"tr_TR": "OS.js Pencere Yöneticisi",
		"vi_VN": "Quản lí cửa sổ OS.js"
	},
	"singular": true,
	"type": "windowmanager",
	"icon": "apps/gnome-window-manager.png",
	"splash": false,
	"preload": [{
		"src": "scheme.html",
		"type": "scheme"
	}, {
		"type": "javascript",
		"src": "combined.js"
	}, {
		"type": "stylesheet",
		"src": "combined.css"
	}],
	"panelItems": {
		"AppMenu": {
			"Name": "AppMenu",
			"Description": "Application Menu",
			"Icon": "actions/stock_about.png",
			"HasOptions": false
		},
		"Buttons": {
			"Name": "Buttons",
			"Description": "Button Bar",
			"Icon": "actions/stock_about.png"
		},
		"Clock": {
			"Name": "Clock",
			"Description": "View the time",
			"Icon": "status/appointment-soon.png",
			"HasOptions": true
		},
		"NotificationArea": {
			"Name": "NotificationArea",
			"Description": "View notifications",
			"Icon": "apps/gnome-panel-notification-area.png"
		},
		"Search": {
			"Name": "Search",
			"Description": "Perform searches",
			"Icon": "actions/find.png",
			"HasOptions": true
		},
		"Weather": {
			"Name": "Weather",
			"Description": "Weather notification",
			"Icon": "status/weather-few-clouds.png"
		},
		"WindowList": {
			"Name": "Window List",
			"Description": "Toggle between open windows",
			"Icon": "apps/xfwm4.png"
		}
	},
	"path": "default/CoreWM",
	"build": {},
	"repo": "default"
},
"default/FileManager": {
	"className": "ApplicationFileManager",
	"name": "File Manager",
	"description": "The default file manager",
	"names": {
		"bg_BG": "Файлов мениджър",
		"de_DE": "Dateimanager",
		"fr_FR": "Explorateur de fichier",
		"it_IT": "Gestore File",
		"nl_NL": "bestands beheer",
		"no_NO": "Fil-håndtering",
		"pl_PL": "Menedżer Plików",
		"ko_KR": "파일 탐색기",
		"sk_SK": "Správca súborov",
		"ru_RU": "Файловый менеджер",
		"tr_TR": "Dosya Yöneticisi",
		"vi_VN": "Quản lí file"
	},
	"descriptions": {
		"bg_BG": "Стандартния файлов мениджър",
		"de_DE": "Standardmäßiger Dateimanager",
		"fr_FR": "Gestionnaire de fichier par défaut",
		"it_IT": "Il gestore file predefinito",
		"nl_NL": "Standaard bestands beheerder",
		"no_NO": "Standard Fil-håndtering program",
		"pl_PL": "Domyślny Menedżer Plików",
		"ko_KR": "기본 파일 관리자",
		"sk_SK": "Štandardný správca súborov",
		"ru_RU": "Стандартный файловый менеджер",
		"tr_TR": "Varsayılan dosya yöneticisi",
		"vi_VN": "Trình quản lí file mặc định"
	},
	"category": "utilities",
	"icon": "apps/file-manager.png",
	"preload": [{
		"type": "javascript",
		"src": "combined.js"
	}, {
		"type": "stylesheet",
		"src": "combined.css"
	}, {
		"src": "scheme.html",
		"type": "scheme"
	}],
	"type": "application",
	"path": "default/FileManager",
	"build": {},
	"repo": "default"
},
"default/MarkOn": {
	"className": "ApplicationMarkOn",
	"name": "MarkOn",
	"mime": [
		"^text",
		"inode\\/x\\-empty",
		"application\\/x\\-empty"
	],
	"category": "office",
	"icon": "apps/libreoffice34-writer.png",
	"preload": [{
		"type": "javascript",
		"src": "combined.js"
	}, {
		"type": "stylesheet",
		"src": "combined.css"
	}, {
		"src": "scheme.html",
		"type": "scheme"
	}],
	"type": "application",
	"path": "default/MarkOn",
	"build": {},
	"repo": "default"
},
"default/PDFjs": {
	"className": "ApplicationPDFjs",
	"name": "PDF Viewer",
	"description": "PDF Viewer",
	"mime": [
		"application/pdf"
	],
	"category": "office",
	"icon": "mimetypes/gnome-mime-application-pdf.png",
	"build": {
		"copy": [
			"metadata.json",
			"scheme.html",
			"main.css",
			"main.js",
			"vendor/pdf.js"
		]
	},
	"preload": [{
		"type": "javascript",
		"src": "combined.js"
	}, {
		"type": "stylesheet",
		"src": "combined.css"
	}, {
		"src": "scheme.html",
		"type": "scheme"
	}],
	"type": "application",
	"path": "default/PDFjs",
	"repo": "default"
},
"default/Preview": {
	"className": "ApplicationPreview",
	"name": "Preview",
	"description": "Preview image files",
	"names": {
		"bg_BG": "Преглед на изображения",
		"de_DE": "Vorschau",
		"fr_FR": "Visionneuse",
		"it_IT": "Anteprima Immagini",
		"ko_KR": "미리보기",
		"nl_NL": "Foto viewer",
		"no_NO": "Forhåndsviser",
		"pl_PL": "Podgląd",
		"ru_RU": "Просмотрщик",
		"sk_SK": "Prehliadač obrázkov",
		"tr_TR": "Önizle",
		"vi_VN": "Trình xem ảnh"
	},
	"descriptions": {
		"bg_BG": "Преглед на изображения",
		"de_DE": "Bildervorschau",
		"fr_FR": "Visionneuse de photos",
		"it_IT": "Anteprima Immagini",
		"ko_KR": "이미지 파일을 미리 봅니다",
		"nl_NL": "Foto viewer",
		"no_NO": "Forhåndsvisning av bilde-filer",
		"pl_PL": "Podgląd zdjęć",
		"ru_RU": "Просмотрщик изображений",
		"sk_SK": "Prehliadač obrázkov",
		"tr_TR": "resim dosyalarını önizle",
		"vi_VN": "Trình xem ảnh"
	},
	"mime": [
		"^image",
		"^video"
	],
	"category": "multimedia",
	"icon": "mimetypes/image.png",
	"preload": [{
		"type": "javascript",
		"src": "combined.js"
	}, {
		"type": "stylesheet",
		"src": "combined.css"
	}, {
		"src": "scheme.html",
		"type": "scheme"
	}],
	"type": "application",
	"path": "default/Preview",
	"build": {},
	"repo": "default"
},
"default/ProcessViewer": {
	"className": "ApplicationProcessViewer",
	"name": "Process Viewer",
	"description": "View running processes",
	"names": {
		"bg_BG": "Процеси",
		"de_DE": "Prozess-Manager",
		"fr_FR": "Gestionnaire de processus",
		"it_IT": "Gestore Attività",
		"ko_KR": "프로세스 관리자",
		"nl_NL": "Proces manager",
		"no_NO": "Prosess oversikt",
		"pl_PL": "Procesy",
		"ru_RU": "Менеджер процессов",
		"sk_SK": "Správca procesov",
		"tr_TR": "İşlemleri Görüntüle",
		"vi_VN": "Xem tiến trình"
	},
	"descriptions": {
		"bg_BG": "Преглед на процеси",
		"de_DE": "Laufende Prozesse verwalten",
		"fr_FR": "Visualiser les processus en cours",
		"it_IT": "Mostri processi attivi",
		"ko_KR": "실행 중인 프로세스를 관리합니다",
		"nl_NL": "Bekijk de lopende processen",
		"no_NO": "Se oversikt over kjørende prosesser",
		"pl_PL": "Zobacz działające procesy",
		"ru_RU": "Менеджер запущенных процессов",
		"sk_SK": "Spravovanie bežiacich procesov",
		"tr_TR": "çalışan işlemleri görüntüle",
		"vi_VN": "Xem các tiến trình đang chạy"
	},
	"singular": true,
	"category": "system",
	"icon": "apps/gnome-monitor.png",
	"preload": [{
		"type": "javascript",
		"src": "combined.js"
	}, {
		"type": "stylesheet",
		"src": "combined.css"
	}, {
		"src": "scheme.html",
		"type": "scheme"
	}],
	"type": "application",
	"path": "default/ProcessViewer",
	"build": {},
	"repo": "default"
},
"default/Settings": {
	"className": "ApplicationSettings",
	"preloadParallel": true,
	"name": "Settings",
	"mime": null,
	"icon": "categories/applications-system.png",
	"category": "system",
	"singular": true,
	"names": {
		"bg_BG": "Настройки",
		"de_DE": "Einstellungen",
		"es_ES": "Settings",
		"fr_FR": "Paramètres",
		"it_IT": "Settaggi",
		"ko_KR": "환경설정",
		"nl_NL": "Instellingen",
		"no_NO": "Instillinger",
		"pl_PL": "Ustawienia",
		"ru_RU": "Настройки",
		"sk_SK": "Nastavenia",
		"tr_TR": "Ayarlar",
		"vi_VN": "Cài đặt"
	},
	"descriptions": {
		"bg_BG": "Настройки",
		"de_DE": "Einstellungen",
		"es_ES": "Settings",
		"fr_FR": "Paramètres",
		"it_IT": "Settaggi",
		"ko_KR": "환경설정",
		"nl_NL": "Instellingen",
		"no_NO": "Instillinger",
		"pl_PL": "Ustawienia",
		"ru_RU": "Настройки",
		"sk_SK": "Nastavenia",
		"tr_TR": "Program Ayarlarını düzenle",
		"vi_VN": "Cài đặt"
	},
	"preload": [{
		"type": "javascript",
		"src": "combined.js"
	}, {
		"type": "stylesheet",
		"src": "combined.css"
	}, {
		"src": "scheme.html",
		"type": "scheme"
	}],
	"type": "application",
	"path": "default/Settings",
	"build": {},
	"repo": "default"
},
"default/Textpad": {
	"className": "ApplicationTextpad",
	"name": "Textpad",
	"description": "Simple text editor",
	"names": {
		"bg_BG": "Текстов редактор",
		"de_DE": "Texteditor",
		"fr_FR": "Éditeur de texte",
		"it_IT": "Editor Testi",
		"ko_KR": "텍스트패드",
		"nl_NL": "Notities",
		"no_NO": "Tekstblokk",
		"pl_PL": "Notatnik",
		"ru_RU": "Редактор текста",
		"sk_SK": "Poznámkový blok",
		"tr_TR": "Basit Bir Metin Düzenleyicisi",
		"vi_VN": "Trình sửa văn bản"
	},
	"descriptions": {
		"bg_BG": "Стандартен текстов редактор",
		"de_DE": "Einfacher Texteditor",
		"fr_FR": "Éditeur de texte simple",
		"it_IT": "Semplice editor di testi",
		"ko_KR": "간단한 텍스트 편집기",
		"nl_NL": "Eenvoudige Tekstverwerker",
		"no_NO": "Simpel tekst redigering",
		"pl_PL": "Prosty edytor tekstu",
		"ru_RU": "Простой текстовый редактор",
		"sk_SK": "Jednoduchý textový editor",
		"tr_TR": "Basit Bir Metin Düzenleyicisi",
		"vi_VN": "Trình sửa văn bản đơn giản"
	},
	"mime": [
		"^text",
		"inode\\/x\\-empty",
		"application\\/x\\-empty",
		"application\\/x\\-lua",
		"application\\/x\\-python",
		"application\\/javascript",
		"application\\/json"
	],
	"category": "utilities",
	"icon": "apps/accessories-text-editor.png",
	"preload": [{
		"type": "javascript",
		"src": "combined.js"
	}, {
		"type": "stylesheet",
		"src": "combined.css"
	}, {
		"src": "scheme.html",
		"type": "scheme"
	}],
	"type": "application",
	"path": "default/Textpad",
	"build": {},
	"repo": "default"
},
"default/wTerm": {
	"className": "ApplicationwTerm",
	"name": "wTerm",
	"mime": null,
	"icon": "apps/terminal.png",
	"category": "system",
	"preload": [{
		"type": "stylesheet",
		"src": "combined.css"
	}, {
		"type": "javascript",
		"src": "combined.js"
	}, {
		"src": "scheme.html",
		"type": "scheme"
	}],
	"type": "application",
	"path": "default/wTerm",
	"build": {},
	"repo": "default"
},
"default/LuaPlayground": {
    "className": "ApplicationLuaPlayground",
    "name": "Lua Playground",
    "mime": null,
    "icon": "categories/preferences-other.png",
    "category": "development",
    "preload": [
        {
            "type": "javascript",
            "src": "combined.js"
        },
        {
            "type": "stylesheet",
            "src": "combined.css"
        },
        {
            "src": "scheme.html",
            "type": "scheme"
        }
    ],
    "type": "application",
    "path": "default/LuaPlayground",
    "build": {},
    "repo": "default"
}