if EXIST C:\Program Files\doxygen\bin\doxygen.exe (
    set CF_PATH=C:\Program Files\doxygen\bin
    set PATH=%CF_PATH%;%PATH%
    set BUILD_DOC_DIR=%KRAKEN_DIR%doc\doxygen
    cd %BUILD_DOC_DIR%
    goto detect_done
)

echo doxygen not found
exit /b 1

:detect_done
@REM echo found doxygen (%CF_PATH%)

if EXIST %PYTHON% (
    set PYTHON=%BUILD_VS_LIBDIR%\python\310\bin\python.exe
    goto detect_python_done
)

echo python not found in lib folder
exit /b 1

:detect_python_done
@REM echo found python (%PYTHON%)

if "%SPHINXBUILD%" == "" (
	set SPHINXBUILD=sphinx-build
)

set ALLSPHINXOPTS=-d %BUILD_DOC_DIR%\doctrees %SPHINXOPTS% source
set I18NSPHINXOPTS=%SPHINXOPTS% source
if NOT "%PAPER%" == "" (
	set ALLSPHINXOPTS=-D latex_paper_size=%PAPER% %ALLSPHINXOPTS%
	set I18NSPHINXOPTS=-D latex_paper_size=%PAPER% %I18NSPHINXOPTS%
)

if "%DOCS_ARGS%" == "" (
  set DOCS_ARGS=html
)

if "%DOCS_ARGS%" == "help" (
	:help
	echo.Please use `make docs ^<target^>` where ^<target^> is one of
	echo.  html       to make standalone HTML files
	echo.  dirhtml    to make HTML files named index.html in directories
	echo.  singlehtml to make a single large HTML file
	echo.  pickle     to make pickle files
	echo.  json       to make JSON files
	echo.  htmlhelp   to make HTML files and a HTML help project
	echo.  qthelp     to make HTML files and a qthelp project
	echo.  devhelp    to make HTML files and a Devhelp project
	echo.  epub       to make an epub
	echo.  latex      to make LaTeX files, you can set PAPER=a4 or PAPER=letter
	echo.  text       to make text files
	echo.  man        to make manual pages
	echo.  texinfo    to make Texinfo files
	echo.  gettext    to make PO message catalogs
	echo.  changes    to make an overview over all changed/added/deprecated items
	echo.  xml        to make Docutils-native XML files
	echo.  pseudoxml  to make pseudoxml-XML files for display purposes
	echo.  linkcheck  to check all external links for integrity
	echo.  doctest    to run all doctests embedded in the documentation if enabled
	echo.  coverage   to run coverage check of the documentation if enabled
	goto EOF
)


if "%DOCS_ARGS%" == "clean" (
	if exist %BUILD_DOC_DIR%\doctrees\ (
		@RD /S /Q %BUILD_DOC_DIR%\doctrees\
	)

	if exist %BUILD_DOC_DIR%\html (
		@RD /S /Q %BUILD_DOC_DIR%\html
	)

	if exist %BUILD_DOC_DIR%\xml (
		@RD /S /Q %BUILD_DOC_DIR%\xml
	)
	echo.
	echo DOCS CLEANED OUT
	echo.
	goto EOF
)

if "%DOCS_ARGS%" == "html" (
	if exist %BUILD_DOC_DIR%\doctrees\ (
		@RD /S /Q %BUILD_DOC_DIR%\doctrees\
	)

	if exist %BUILD_DOC_DIR%\html (
		@RD /S /Q %BUILD_DOC_DIR%\html
	)

	if exist %BUILD_DOC_DIR%\xml (
		@RD /S /Q %BUILD_DOC_DIR%\xml
	)
)


REM Check if sphinx-build is available and fallback to Python version if any
%SPHINXBUILD% 1>NUL 2>NUL
if errorlevel 9009 goto sphinx_python
goto sphinx_ok

:sphinx_python

set SPHINXBUILD=%PYTHON% -B -m sphinx.__init__
%SPHINXBUILD% 2> nul
if errorlevel 9009 (
  %PYTHON% -m pip install Sphinx
	%PYTHON% -m pip install sphinx-rtd-theme
	goto sphinx_ok
)

:sphinx_ok


if "%DOCS_ARGS%" == "update" (
	echo.
	echo UPDATING DOCUMENTATION
	%SPHINXBUILD% -b html %ALLSPHINXOPTS% %BUILD_DOC_DIR%\html
	if errorlevel 1 exit /b 1
	echo.
	echo.Success. View the site locally by opening this file in your web browser:
	echo.
	echo.%BUILD_DOC_DIR%\html\index.html
	echo.
	goto EOF
)


if "%DOCS_ARGS%" == "html" (
	echo.
	echo BUILDING KRAKEN DOCUMENTATION WITH DOXYGEN AND SPHINX
  doxygen
	%SPHINXBUILD% -b html %ALLSPHINXOPTS% %BUILD_DOC_DIR%\html
	if errorlevel 1 exit /b 1
	echo.
	echo.Success. View the site locally by opening this file in your web browser:
	echo.
	echo.%BUILD_DOC_DIR%\html\index.html
	echo.
	goto EOF
)

if "%DOCS_ARGS%" == "dirhtml" (
	%SPHINXBUILD% -b dirhtml %ALLSPHINXOPTS% %BUILD_DOC_DIR%\dirhtml
	if errorlevel 1 exit /b 1
	echo.
	echo.Build finished. The HTML pages are in %BUILD_DOC_DIR%\dirhtml.
	goto EOF
)

if "%DOCS_ARGS%" == "singlehtml" (
	%SPHINXBUILD% -b singlehtml %ALLSPHINXOPTS% %BUILD_DOC_DIR%\singlehtml
	if errorlevel 1 exit /b 1
	echo.
	echo.Build finished. The HTML pages are in %BUILD_DOC_DIR%\singlehtml.
	goto EOF
)

if "%DOCS_ARGS%" == "pickle" (
	%SPHINXBUILD% -b pickle %ALLSPHINXOPTS% %BUILD_DOC_DIR%\pickle
	if errorlevel 1 exit /b 1
	echo.
	echo.Build finished; now you can process the pickle files.
	goto EOF
)

if "%DOCS_ARGS%" == "json" (
	%SPHINXBUILD% -b json %ALLSPHINXOPTS% %BUILD_DOC_DIR%\json
	if errorlevel 1 exit /b 1
	echo.
	echo.Build finished; now you can process the JSON files.
	goto EOF
)

if "%DOCS_ARGS%" == "htmlhelp" (
	%SPHINXBUILD% -b htmlhelp %ALLSPHINXOPTS% %BUILD_DOC_DIR%\htmlhelp
	if errorlevel 1 exit /b 1
	echo.
	echo.Build finished; now you can run HTML Help Workshop with the ^
.hhp project file in %BUILD_DOC_DIR%\htmlhelp.
	goto EOF
)

if "%DOCS_ARGS%" == "qthelp" (
	%SPHINXBUILD% -b qthelp %ALLSPHINXOPTS% %BUILD_DOC_DIR%\qthelp
	if errorlevel 1 exit /b 1
	echo.
	echo.Build finished; now you can run "qcollectiongenerator" with the ^
.qhcp project file in %BUILD_DOC_DIR%\qthelp, like this:
	echo.^> qcollectiongenerator %BUILD_DOC_DIR%\qthelp\packagename.qhcp
	echo.To view the help file:
	echo.^> assistant -collectionFile %BUILD_DOC_DIR%\qthelp\packagename.ghc
	goto EOF
)

if "%DOCS_ARGS%" == "devhelp" (
	%SPHINXBUILD% -b devhelp %ALLSPHINXOPTS% %BUILD_DOC_DIR%\devhelp
	if errorlevel 1 exit /b 1
	echo.
	echo.Build finished.
	goto EOF
)

if "%DOCS_ARGS%" == "epub" (
	%SPHINXBUILD% -b epub %ALLSPHINXOPTS% %BUILD_DOC_DIR%\epub
	if errorlevel 1 exit /b 1
	echo.
	echo.Build finished. The epub file is in %BUILD_DOC_DIR%\epub.
	goto EOF
)

if "%DOCS_ARGS%" == "latex" (
	%SPHINXBUILD% -b latex %ALLSPHINXOPTS% %BUILD_DOC_DIR%\latex
	if errorlevel 1 exit /b 1
	echo.
	echo.Build finished; the LaTeX files are in %BUILD_DOC_DIR%\latex.
	goto EOF
)

if "%DOCS_ARGS%" == "latexpdf" (
	%SPHINXBUILD% -b latex %ALLSPHINXOPTS% %BUILD_DOC_DIR%\latex
	cd %BUILD_DOC_DIR%\latex
	make all-pdf
	cd %~dp0
	echo.
	echo.Build finished; the PDF files are in %BUILD_DOC_DIR%\latex.
	goto EOF
)

if "%DOCS_ARGS%" == "latexpdfja" (
	%SPHINXBUILD% -b latex %ALLSPHINXOPTS% %BUILD_DOC_DIR%\latex
	cd %BUILD_DOC_DIR%\latex
	make all-pdf-ja
	cd %~dp0
	echo.
	echo.Build finished; the PDF files are in %BUILD_DOC_DIR%\latex.
	goto EOF
)

if "%DOCS_ARGS%" == "text" (
	%SPHINXBUILD% -b text %ALLSPHINXOPTS% %BUILD_DOC_DIR%\text
	if errorlevel 1 exit /b 1
	echo.
	echo.Build finished. The text files are in %BUILD_DOC_DIR%\text.
	goto EOF
)

if "%DOCS_ARGS%" == "man" (
	%SPHINXBUILD% -b man %ALLSPHINXOPTS% %BUILD_DOC_DIR%\man
	if errorlevel 1 exit /b 1
	echo.
	echo.Build finished. The manual pages are in %BUILD_DOC_DIR%\man.
	goto EOF
)

if "%DOCS_ARGS%" == "texinfo" (
	%SPHINXBUILD% -b texinfo %ALLSPHINXOPTS% %BUILD_DOC_DIR%\texinfo
	if errorlevel 1 exit /b 1
	echo.
	echo.Build finished. The Texinfo files are in %BUILD_DOC_DIR%\texinfo.
	goto EOF
)

if "%DOCS_ARGS%" == "gettext" (
	%SPHINXBUILD% -b gettext %I18NSPHINXOPTS% %BUILD_DOC_DIR%\locale
	if errorlevel 1 exit /b 1
	echo.
	echo.Build finished. The message catalogs are in %BUILD_DOC_DIR%\locale.
	goto EOF
)

if "%DOCS_ARGS%" == "changes" (
	%SPHINXBUILD% -b changes %ALLSPHINXOPTS% %BUILD_DOC_DIR%\changes
	if errorlevel 1 exit /b 1
	echo.
	echo.The overview file is in %BUILD_DOC_DIR%\changes.
	goto EOF
)

if "%DOCS_ARGS%" == "linkcheck" (
	%SPHINXBUILD% -b linkcheck %ALLSPHINXOPTS% %BUILD_DOC_DIR%\linkcheck
	if errorlevel 1 exit /b 1
	echo.
	echo.Link check complete; look for any errors in the above output ^
or in %BUILD_DOC_DIR%\linkcheck/output.txt.
	goto EOF
)

if "%DOCS_ARGS%" == "doctest" (
	%SPHINXBUILD% -b doctest %ALLSPHINXOPTS% %BUILD_DOC_DIR%\doctest
	if errorlevel 1 exit /b 1
	echo.
	echo.Testing of doctests in the sources finished, look at the ^
results in %BUILD_DOC_DIR%\doctest/output.txt.
	goto EOF
)

if "%DOCS_ARGS%" == "coverage" (
	%SPHINXBUILD% -b coverage %ALLSPHINXOPTS% %BUILD_DOC_DIR%\coverage
	if errorlevel 1 exit /b 1
	echo.
	echo.Testing of coverage in the sources finished, look at the ^
results in %BUILD_DOC_DIR%\coverage/python.txt.
	goto EOF
)

if "%DOCS_ARGS%" == "xml" (
	%SPHINXBUILD% -b xml %ALLSPHINXOPTS% %BUILD_DOC_DIR%\xml
	if errorlevel 1 exit /b 1
	echo.
	echo.Build finished. The XML files are in %BUILD_DOC_DIR%\xml.
	goto EOF
)

if "%DOCS_ARGS%" == "pseudoxml" (
	%SPHINXBUILD% -b pseudoxml %ALLSPHINXOPTS% %BUILD_DOC_DIR%\pseudoxml
	if errorlevel 1 exit /b 1
	echo.
	echo.Build finished. The pseudo-XML files are in %BUILD_DOC_DIR%\pseudoxml.
	goto EOF
)

:EOF