(progn
  (or (equal major-mode 'c++-mode)
      (c++-mode))
  (setq c-hanging-braces-alist (quote ((block-close before) (statement-cont) (substatement-open after) (brace-list-open) (brace-entry-open) (extern-lang-open after) (namespace-open after)  (module-open after) (composition-open after) (inexpr-class-open after) (inexpr-class-close before) (arglist-cont-nonempty))))
  (add-hook 'before-save-hook 'delete-trailing-whitespace t t)
  (setq program-name "torrent-file-editor")
  (setq program-subdir "linux"))
