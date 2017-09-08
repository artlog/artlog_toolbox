
(defun simplified-beginning-of-buffer ()
  "Move point to the beginning of the buffer;
leave mark at previous position."
  (interactive)
  (push-mark)
  (goto-char (point-min)))

(defun jump-to-mark ()
  (interactive)
  (goto-char (mark-marker))
  (pop-mark)
)

(defvar al-menu-bar-menu (make-sparse-keymap "Mine"))

(defun remove-artlog-menuf()
  "remove ArtLog al-menu-bar-menu"
  (message "removing ArtLog menu.")
  (define-key al-menu-bar-menu [al-cmd1] nil)
  (define-key al-menu-bar-menu [al-cmd2] nil)
  (define-key al-menu-bar-menu [remove-artlog-menu] nil)
  (define-key global-map [menu-bar my-menu] nil)
)

(defun create-artlog-menu ()
  "Create Artlog Menu"
  (define-key global-map [menu-bar my-menu] (cons "ArtLog" al-menu-bar-menu))
  (define-key al-menu-bar-menu [al-rm]
    '(menu-item "Remove Menu" remove-artlog-menu :help "Remove Artlog menu"))
  (define-key al-menu-bar-menu [al-cmd1]
    '(menu-item "Goto Beginning" simplified-beginning-of-buffer :help "Do what al-cmd1 does"))
  (define-key al-menu-bar-menu [al-cmd2]
    '(menu-item "Go back to mark" jump-to-mark :help "Do what al-cmd1 does"))
  (define-key al-menu-bar-menu [al-terminal]
    '(menu-item "Launch terminal" term-doit :help "Launch terminal within emacs"))

)

(defun remove-artlog-menu ()
  (interactive)
  (remove-artlog-menuf))

(create-artlog-menu)

(defun term-doit ()
  (interactive)
  (ansi-term "./doit.sh")
)

