cat <<EOF >> ~/.vimrc
set hlsearch
set nu
set autoindent
set scrolloff=2
set wildmode=longest,list
set ts=4
set sts=4
set sw=1
set autowrite
set autoread
set cindent
set bs=eol,start,indent
set history=256
set laststatus=2
set paste
set shiftwidth=4
set showmatch
set smartcase
set smarttab
set smartindent
set softtabstop=4
set tabstop=4
set ruler
set incsearch
set statusline=\ %<%l:%v\ [%P]%=%a\ %h%m%r\ %F\
 :hi CursorLine cterm=NONE ctermbg=yellow ctermfg=white guibg=yellow guifg=white
 :hi CursorColumn cterm=NONE ctermbg=yellow ctermfg=white guibg=yellow guifg=white
 augroup CursorLine
     au!
     au VimEnter,WinEnter,BufWinEnter * setlocal cursorline
     au WinLeave * setlocal nocursorline
 augroup END
 EOF
