"""Fetch dependent repositories defined in repos.json."""

import json
import os
import subprocess
from pathlib import Path


def _run_git(args):
    subprocess.run(['git', *args], check=True)


def _maybe_update_submodules(repo_path: Path) -> None:
    if (repo_path / '.gitmodules').is_file():
        _run_git(['-C', str(repo_path), 'submodule', 'update', '--init', '--recursive'])


def clone_or_update_repo(repo_url, path, branch):
    repo_path = Path(path)

    if repo_path.is_dir():
        _run_git(['-C', str(repo_path), 'fetch', '--prune'])

        if branch:
            _run_git(['-C', str(repo_path), 'checkout', branch])

            remote_ref = subprocess.run(
                ['git', '-C', str(repo_path), 'show-ref', '--verify', f'refs/remotes/origin/{branch}'],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                check=False,
            )

            if remote_ref.returncode == 0:
                _run_git(['-C', str(repo_path), 'reset', '--hard', f'origin/{branch}'])
        else:
            _run_git(['-C', str(repo_path), 'pull', '--ff-only'])

        _maybe_update_submodules(repo_path)
        return

    command = ['clone']

    if branch:
        command.extend(['-b', branch])

    command.extend([repo_url, str(repo_path)])

    _run_git(command)
    _maybe_update_submodules(repo_path)

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    config_path = os.path.join(script_dir, 'repos.json')

    with open(config_path) as f:
        repos = json.load(f)

    for repo in repos:
        repo_path = os.path.join(script_dir, repo['path'])
        branch = repo.get('branch')
        clone_or_update_repo(repo['url'], repo_path, branch)


if __name__ == "__main__":
    print("start fetching dependent repos..")
    main()
