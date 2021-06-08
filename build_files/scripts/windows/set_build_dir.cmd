if NOT "%TARGET%" == "" (
    set BUILD_DIR=%BUILD_DIR%_%TARGET%
) else (
    set BUILD_DIR=%BUILD_DIR%_%BUILD_TYPE%
)

if NOT "%BUILD_DIR_OVERRRIDE%"=="" (
	set BUILD_DIR=%BUILD_DIR_OVERRRIDE%
)