for project_name in inst malloc rbt
    do
        python3 ./cmd.py --build $project_name
        python3 ./cmd.py --test $project_name
    done 