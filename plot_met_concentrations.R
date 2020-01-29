library(ggplot2)
library(reshape2)

stochastic = TRUE; #to switch between deterministic or stochastic version
steps = FALSE;      #for the stochastic version only: to choose if print based on time or steps
fix_ack = FALSE;    #to choose if we want to plot increase and decrease at the same time or acording to simulation time misrepresented by ack state

#If only one metabolite has too high concentration:
#df = df[, -xxx] # where xxx is the index of the variable (or a list of variables) to be not shown in the plot
#decomment scale_y_continuous(limit = c(0, xxx)) where xxx is the upper limit of y axis to better shows the bottom part of the plot

#If the plot is "compressed" on the left:
#decomment scale_x_continuous(limit = c(0, xxx)) where xxx is the limit of x axis

#Information on melt:
#https://stackoverflow.com/questions/25317440/rggplot2-get-both-columns-on-one-plot

df = read.csv(file = "met_concentrations.txt", sep = ",", header = TRUE)

if (stochastic) {
  df = df[, -ncol(df)] #because there are only commas

  if (steps) {
    df = df[,-2] #remove useless time column
    merged = melt(df, id = "steps")
    colnames(merged) = c("Steps", "Metabolites", "Concentration")
    plot = ggplot(merged, aes(x = Steps, y = Concentration, color = Metabolites))

  } else {
    df = df[,-1] #remove useless steps column
    merged = melt(df, id = "time")
    colnames(merged) = c("Time", "Metabolites", "Concentration")
    plot = ggplot(merged, aes(x = Time, y = Concentration, color = Metabolites))

  }
} else {

  df[,1] = gsub(".{3}$", "", df[,1])             #remove (3 from the end) time unit (and use as a integer in the next steps)
  df[,2] = gsub("metabolite_", "", df[,2])       #remove 'metabolite_' prefix

  if (fix_ack) { #sum and consider at same time increasing and decreasing of concentration
    df[,1] = as.integer(unname(sapply(df[,1], function(x) eval(parse(text=x)))))
  } else {
    df[,1] = as.integer(gsub(".*\\+", "", df[,1]))
  }

  plot = ggplot(df, aes(x = Time, y = Concentration, color = Metabolite))

}
plot = plot +
  #scale_x_continuous(limit = c(0, xxx)) +
  #scale_y_continuous(limit = c(0, xxx)) +
  #stat_smooth() +
  #theme(legend.position="none") +
  geom_line(size = 1.2)
#plot

#export to png image
ggsave("met_concentrations_Rplot.png", plot)
