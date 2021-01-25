#!/bin/bash
# sets the properties of ci releases
# this doesn't have to be 100% foolproof
# the releases are first made as drafts and will be vetted by a human
# it just has to provide a template
# this requires the repo to be unshallowed
beta_regex='beta'
name_regex='set\(GIT_TAG_RELEASENAME "([[:print:]]+)")'

if [[ $1 ]]; then
  TAG="$1"
fi

# check env
if [[ ! $TAG ]]; then
  echo "::error file=$0::TAG is missing"
  exit 2
fi

# create title
if [[ $TAG =~ $beta_regex ]]; then
  echo "::set-output name=is_beta::yes"
  title="$TAG"
  echo "creating beta release '$title'"
elif [[ ! $(cat CMakeLists.txt) =~ $name_regex ]]; then
  echo "::error file=$0::could not find releasename in CMakeLists.txt"
  exit 1
else
  echo "::set-output name=is_beta::no"
  name="${BASH_REMATCH[1]}"
  version="${TAG##*-}"
  title="Cockatrice $version: $name"
  no_beta=1
  echo "::set-output name=friendly_name::$name"
  echo "creating full release '$title'"
fi
echo "::set-output name=title::$title"

# add release notes template
if [[ $no_beta ]]; then
  body="$(cat ".ci/release_template.md")"
  if [[ ! $body ]]; then
    echo "::warning file=$0::could not find release template"
  fi
  body="${body//__REPLACE_WITH_RELEASE_TITLE__/$title}"
else
  body="Included commits over previous version:
__REPLACE_WITH_GENERATED_LIST__"
fi

# add git log to release notes
all_tags="
$(git tag)" # tags are ordered alphabetically
before="${all_tags%%
$TAG*}" # strip line with current tag an all lines after it
# note the extra newlines are needed to always have a last line
if [[ $all_tags == $before ]]; then
  echo "::warning file=$0::could not find current tag"
else
  while
    previous="${before##*
}" # get the last line
    # skip this tag if this is a full release and it's a beta or if empty
    [[ $no_beta && $previous =~ $beta_regex || ! $previous ]]
  do
    beta_list+=" $previous" # add to list of skipped betas
    next_before="${before%
*}" # strip the last line
    if [[ $next_before == $before ]]; then
      echo "::warning file=$0::could not find previous tag"
      unset previous
      break
    fi
    before="$next_before"
  done
  if [[ $previous ]]; then
    if ! generated_list="$(git log "$previous..$TAG" --pretty="- %s")"; then
      echo "::warning file=$0::failed to produce git log"
    fi
    body="${body//__REPLACE_WITH_GENERATED_LIST__/$generated_list}"
    body="${body//__REPLACE_WITH_BETA_LIST__/$beta_list}"
  fi
fi
echo "::set-output name=body::$body"
