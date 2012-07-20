require 'proc'

print('========== proc.list() ==========')
t = proc.list()
for i, k in ipairs(t) do
    print(i, k.pid, k.name)
end

print('========== proc.pidof() ==========')
p1 = proc.pidof('init')
print('pidof(init) = ')
for _, v in ipairs(p1) do
    print('\t' .. v)
end
p2 = proc.pidof('bash')
print('pidof(bash) = ')
for _, v in ipairs(p2) do
    print('\t' .. v)
end

