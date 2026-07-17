# Openbox Translations - Meson Build

# i18n module
i18n = import('i18n')

# Get list of languages from LINGUAS file
linguas_file = files('LINGUAS')

# Read LINGUAS content
linguas_content = run_command('cat', 'LINGUAS', check : false).stdout().strip()

# Parse languages
languages = []
foreach line : linguas_content.split('\n')
  stripped = line.strip()
  if stripped != '' and not stripped.startswith('#')
    languages += stripped
  endif
endforeach

# Generate translations
i18n.gettext(gettext_package,
  languages : languages,
  args : [
    '--directory=' + meson.source_root(),
    '--from-code=UTF-8',
  ],
)
