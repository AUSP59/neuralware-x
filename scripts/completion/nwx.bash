# SPDX-License-Identifier: Apache-2.0
# bash completion for nwx
_nwx() {
  COMPREPLY=()
  local cur=${COMP_WORDS[COMP_CWORD]}
  COMPREPLY=( $( compgen -W "--help --config --dataset --epochs --lr --hidden --seed --save --load --eval --optimizer --wd --batch --val_split --patience --standardize --predict --predict_out --history --activation --calibrate_temp --calibration_metric" -- "$cur" ) )
}
complete -F _nwx nwx
