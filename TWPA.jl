using JosephsonCircuits
using Plots
using DelimitedFiles

function main()
    @variables Rleft Rright Cg Lj Cj Cc Cr Lr Cn
    circuit = Tuple{String,String,String,Num}[]

    # port on the input side
    push!(circuit, ("P$(1)_$(0)", "1", "0", 1))
    push!(circuit, ("R$(1)_$(0)", "1", "0", Rleft))
    # first half cap to ground
    push!(circuit, ("C$(1)_$(0)", "1", "0", Cg))
    # middle caps and jj's
    push!(circuit, ("Lj$(1)_$(2)", "1", "2", Lj))
    push!(circuit, ("C$(1)_$(2)", "1", "2", Cj))

    Nj = 2400
    pmrpitch = 3
    j = 2
    
    for i in 2:Nj-1
        if mod(i, pmrpitch) == pmrpitch ÷ 3
            push!(circuit, ("C$(j)_$(0)", "$(j)", "0", Cn))
            push!(circuit, ("Lj$(j)_$(j+2)", "$(j)", "$(j+2)", Lj))
            push!(circuit, ("C$(j)_$(j+2)", "$(j)", "$(j+2)", Cj))

            push!(circuit, ("C$(j)_$(j+1)", "$(j)", "$(j+1)", Cc))
            push!(circuit, ("C$(j+1)_$(0)", "$(j+1)", "0", Cr))
            push!(circuit, ("L$(j+1)_$(0)", "$(j+1)", "0", Lr))

            j += 1
        else
            push!(circuit, ("C$(j)_$(0)", "$(j)", "0", Cg))
            push!(circuit, ("Lj$(j)_$(j+1)", "$(j)", "$(j+1)", Lj))
            push!(circuit, ("C$(j)_$(j+1)", "$(j)", "$(j+1)", Cj))
        end
        j += 1
    end

    # last elements
    push!(circuit, ("R$(j)_$(0)", "$(j)", "0", Rright))
    push!(circuit, ("P$(j)_$(0)", "$(j)", "0", 2))

    circuitdefs = Dict(
        Lj => IctoLj(6.4e-6),  #IctoLjの引数は臨界電流値. それに応じたジョセフソンインダクタンスを返す.
        Cg => 25.0e-15,
        Cc => 18.0e-15,
        Cn => 6.001e-15,
        Cr => 5.00e-12,
        Lr => 65e-12,
        Cj => 164e-15,
        Rleft => 50.0,
        Rright => 50.0,
    )

    ws  = 2π * (1.0:0.1:16) * 1e9    #ここで周波数(横軸を変更): (開始値:ステップ幅:終了値)
    wp  = (2π*8.7*1e9,)
    Ip  = 3.6001e-6
    sources = [(mode=(1,), port=1, current=Ip)]
    Npumpharmonics       = (20,)
    Nmodulationharmonics = (10,)

    @time rpm = hbsolve(ws, wp, sources, Nmodulationharmonics,
    Npumpharmonics, circuit, circuitdefs)

    # プロット
    p1=plot(ws/(2*pi*1e9),
        10*log10.(abs2.(rpm.linearized.S(
                outputmode=(0,),
                outputport=2,
                inputmode=(0,),
                inputport=1,
                freqindex=:),
        )),
        ylim=(-40,30),label="S21",
        xlabel="Signal Frequency (GHz)",
        legend=:bottomright,
        title="Scattering Parameters",
        ylabel="dB")

        plot!(ws/(2*pi*1e9),
            10*log10.(abs2.(rpm.linearized.S((0,),1,(0,),2,:))),
            label="S12",
            )

        plot!(ws/(2*pi*1e9),
            10*log10.(abs2.(rpm.linearized.S((0,),1,(0,),1,:))),
            label="S11",
            )

        plot!(ws/(2*pi*1e9),
            10*log10.(abs2.(rpm.linearized.S((0,),2,(0,),2,:))),
            label="S22",
            )

    p2 = plot(ws/(2π*1e9),
        rpm.linearized.QE((0,),2,(0,),1,:) ./
        rpm.linearized.QEideal((0,),2,(0,),1,:),
        ylim=(0,1.05), xlabel="GHz", ylabel="QE/QEideal",
        legend=false, title="Quantum Efficiency")

    p3=plot(ws/(2*pi*1e9),
        10*log10.(abs2.(rpm.linearized.S(:,2,(0,),1,:)')),
        ylim=(-40,30),
        xlabel="Signal Frequency (GHz)",
        legend=false,
        title="All idlers",
        ylabel="dB")
    
    p4=plot(ws/(2*pi*1e9),
        1 .- rpm.linearized.CM((0,),2,:),    
        legend=false,title="Commutation \n relation error",
        ylabel="Commutation \n relation error",xlabel="Signal Frequency (GHz)");
    
    plot(p1, p2, p3, p4, layout = (2, 2))

    freq = ws/(2*pi*1e9)
    gain = 10*log10.(abs2.(rpm.linearized.S(
                outputmode=(0,),
                outputport=2,
                inputmode=(0,),
                inputport=1,
                freqindex=:)))
    fieldnames(typeof(gain))

    # データを書き出し
    writedlm("freq_gain_sim.txt", ws/(2π*1e9))
    writedlm("trans_gain_sim.txt", 10*log10.(abs2.(rpm.linearized.S((0,),2,(0,),1,:))))
end

function plot_gain()
    x = readdlm("freq_gain_sim.txt") |> vec
    y = readdlm("trans_gain_sim.txt") |> vec

    # データ長をチェック（オプション）
    if length(x) != length(y)
        error("There is a difference between x and y. ", length(x), " ≠ ", length(y))
    end

    # プロット
    plt = plot(
        x, y,
        xlabel = "Frequency [GHz]",
        ylabel = "Gain [dB]",
        legend = false,
        seriestype = :line,    # 線グラフ専用で描く
        marker      = :none,   # マーカー（点）を完全にオフ
        line   = (:solid, 2),
    )

    # 画面表示
    display(plt)

    # ファイルに保存する場合
    savefig(plt, "gain.png")
end


# スクリプト実行時に main() を呼ぶ
if abspath(PROGRAM_FILE) == @__FILE__
    main()
    plot_gain()
end
