# Validate MetaInfo file
metainfo_file = 'io.github.iptux_src.iptux.metainfo.xml'
ascli_exe = find_program('appstreamcli', required: false)
if ascli_exe.found()
  test('validate metainfo file',
        ascli_exe,
        args: ['validate',
               '--no-net',
               '--pedantic',
               metainfo_file]
  )
endif


# Localize a MetaInfo file and install it
i18n = import('i18n')

i18n.merge_file(
    input:  metainfo_file,
    output: 'io.github.iptux_src.iptux.metainfo.xml',
    type: 'xml',
    po_dir: join_paths (meson.source_root(), 'po'),
    install: true,
    install_dir: join_paths (get_option ('datadir'), 'metainfo')
)
