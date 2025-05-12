def puzzle(w, n):
    for i in range(n - 1, len(w), n):
        w = w[:i] + '_' + w[i + 1:]
    print(w)

puzzle(input(), int(input()))
