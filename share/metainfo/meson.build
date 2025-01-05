metainfo_file = files('io.github.iptux_src.iptux.metainfo.xml')
ascli_exe = find_program('appstreamcli', required: true)
sed_exe = find_program('sed', required: true)

metainfo_with_relinfo = custom_target(
  'gen-output',
  input: ['../../NEWS', metainfo_file[0]],
  output: ['nol10n_withrelinfo_io.github.iptux_src.iptux.metainfo.xml'],
  command: [
    ascli_exe,
    'news-to-metainfo',
    '--limit=6', '@INPUT0@',
    '@INPUT1@',
    '@OUTPUT@',
  ],
)



# Localize a MetaInfo file and install it
i18n = import('i18n')

metainfo_i18n = i18n.merge_file(
  input: metainfo_with_relinfo,
  output: 'io.github.iptux_src.iptux-with-icon.metainfo.xml',
  type: 'xml',
  po_dir: meson.source_root() / 'po',
)

desktop_file = custom_target(
  'gen-desktop',
  input: metainfo_i18n,
  output: ['io.github.iptux_src.iptux.desktop'],
  command: [ascli_exe, 'make-desktop-file', '@INPUT@', '@OUTPUT@'],
  install: true,
  install_dir: join_paths(get_option('datadir'), 'applications'),
)

metainfo = custom_target(
  'gen-metainfo',
  input: metainfo_i18n,
  output: ['io.github.iptux_src.iptux.metainfo.xml'],
  capture: true,
  command: [sed_exe, '/icon/d', '@INPUT@'],
  install: true,
  install_dir: get_option('datadir') / 'metainfo',
)

# Validate MetaInfo file
test(
  'validate metainfo file',
  ascli_exe,
  args: ['validate', '--no-net', '--pedantic', metainfo_i18n],
)

