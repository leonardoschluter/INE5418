package gartic_in_java;

import org.jgroups.*;
import org.jgroups.util.Util;

import java.io.*;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;

public class Chat  implements Receiver {
    JChannel channel;
    final List<String> state = new LinkedList<String>();
    private Jartic controller;

    // MESSAGES
    static final String MSG_warningStartRound = " ||| WARNING ||| The round will start in few seconds ... ";
    static final String MSG_gotItRight = "Congratulations, you did get it right !";
    static final String MSG_wordToDraw = "WORD TO DRAW-";
    static final String MSG_roundStarted = "ROUND STARTED-";
    static final String MSG_roundFinished = "ROUND FINISHED - PROCESSING -";
    static final String MSG_newLeader = "NEW_LEADER:";
    static final String MSG_playerAdded = "PLAYER_ADDED:";
    static final String MSG_updateScore = "UPDATE_SCORE:";

    public Chat(Jartic jartic) {
        this.controller = jartic;
        this.channel = jartic.channel;
    }

    public void viewAccepted(View new_view) {
        System.out.println("** view: " + new_view);
        controller.updatePlayersInGame();
    }

    public void receive(Message msg) {
        String line = msg.getObject();
        if(line.contains(MSG_updateScore) && !controller.isLeader()){
            String[] data = line.split(":", 2);
            System.out.println("The new score is -> " + data[1]);
            controller.onUpdateScoreByText(data[1]);
            return;
        }
        if (line.contains(MSG_newLeader)) {
            String newLeader = line.split(":")[1];
            System.out.println("The new player to draw is: " + newLeader);
            controller.onNewLeader(newLeader);
            return;
        }
        if (line.contains(MSG_roundStarted)) {
            Date ts = new Date(Long.parseLong(line.split("-")[1]));
            System.out.println("Round started at: " + ts + " - finishing in 90 seconds ...");
            //TODO should call some controller method
            return;
        }
        if (line.contains(MSG_warningStartRound) || line.contains(MSG_gotItRight) || line.contains(MSG_roundFinished)) {
            System.out.println(line);
            return;
        }

        System.out.println(line);
        System.out.println("ISleader -> "+controller.isLeader());
        if(controller.isLeader()){
            controller.onAnswerSubmitted(line);
        }
        return;
    }

    public void getState(OutputStream output) throws Exception {
        synchronized (state) {
            Util.objectToStream(state, new DataOutputStream(output));
        }
    }

    @SuppressWarnings("unchecked")
    public void setState(InputStream input) throws Exception {
        List<String> list = (List<String>) Util.objectFromStream(new DataInputStream(input));
        synchronized (state) {
            state.clear();
            state.addAll(list);
        }

    }

    public void send(String msgText){
        try {
            Message msg = new ObjectMessage(null, msgText);
            if(channel == null){
                channel = controller.channel;
            }
            channel.send(msg);
        }catch(Exception e){
            e.printStackTrace();
            System.out.println("Failed in sending msg->"+msgText);
        }
    }


    public void eventLoop() {
        BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
        while (true) {
            try {
                System.out.print("> ");
                System.out.flush();
                String line = in.readLine().toLowerCase();
                if (line.startsWith("quit") || line.startsWith("exit")) {
                    break;
                }
                if (line.equals("/start") && controller.canStartRound()) {
                    controller.startRound();
                }else if (line.equals("/score")) {
                    System.out.println(controller.score.toString());
                } else {
                    line = channel.getAddressAsString() + ":" + line;
                    send(line);
                }
            } catch (Exception e) {
            }
        }
    }
}
