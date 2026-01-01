using JosephsonCircuits, Symbolics, Plots, DelimitedFiles, LinearAlgebra

# 最小構成で1回だけ実行してコンパイルさせる
function precompile_work()
    @variables R L C
    circuit = [("P1_0", "1", "0", 1), ("R1_0", "1", "0", R)]
    circuitdefs = Dict(R => 50.0)
    ws = 2π * (1.0:1.0:2.0) * 1e9
    wp = (2π*8e9,)
    sources = [(mode=(1,), port=1, current=1e-6)]
    
    # 計算の実行
    hbsolve(ws, wp, sources, (1,), (1,), circuit, circuitdefs)
    
    # プロットの実行
    p = plot(rand(10))
    savefig(p, "temp.png")
    rm("temp.png")
end

precompile_work()