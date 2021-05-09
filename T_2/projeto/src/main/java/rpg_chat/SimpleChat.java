package rpg_chat;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.jgroups.*;
import org.jgroups.util.Util;

import java.io.*;
import java.util.*;
import java.util.stream.Collectors;

public class SimpleChat implements Receiver {
    JChannel channel;
    Address leader = null;
    String wordToDraw = "";
    Score score = new Score();
    Queue<Answer> roundHistory = new LinkedList<Answer>();
    final List<String> state = new LinkedList<String>();
    private MyDraw draw;

    // MESSAGES
    static final String MSG_warningStartRound = " ||| WARNING ||| The round will start in few seconds ... ";
    static final String MSG_gotItRight = "Congratulations, you did get it right !";
    static final String MSG_wordToDraw = "WORD TO DRAW-";
    static final String MSG_roundStarted = "ROUND STARTED-";
    static final String MSG_roundFinished = "ROUND FINISHED - PROCESSING -";
    static final String MSG_newLeader = "NEW_LEADER:";
    static final String MSG_playerAdded = "PLAYER_ADDED:";
    static final String MSG_updateScore = "UPDATE_SCORE:";

    public void viewAccepted(View new_view) {
        System.out.println("** view: " + new_view);
    }

    public void receive(Message msg) {
        String line = msg.getObject();
        synchronized (state) {
            state.add(line);
        }
        if(line.contains(MSG_updateScore)){
            String[] data = line.split(":", 2);
            score.updateScoreByString(data[1]);
        }
        if(line.contains(MSG_playerAdded)){
            String newPlayer = line.split(":")[1];
            Address playerAddress = findAddressByString(newPlayer);
            score.addPlayer(playerAddress);
        }
        if (line.contains(MSG_newLeader)) {
            String newLeader = line.split(":")[1];
            System.out.println("The new player to draw is: " + newLeader);
            this.leader = findAddressByString(newLeader);
            if (isLeader()) {
                draw.setCanDraw(true);
                System.out.println("You are the next to draw ! type /start to draw");
            } else {
                draw.setCanDraw(false);
            }
            return;
        }
        if (line.contains(MSG_roundStarted)) {
            Date ts = new Date(Long.parseLong(line.split("-")[1]));
            System.out.println("Round started at: " + ts + " - finishing in 90 seconds ...");
            return;
        }
        if (line.contains(MSG_warningStartRound) || line.contains(MSG_gotItRight) || line.contains(MSG_roundFinished)) {
            System.out.println(line);
            return;
        }
        String[] answer = line.split(":");
        if (isLeader() && wordToDraw.equals(answer[1])) {
            System.out.println("Submitted Answer:" + answer[1] + " - Addr: " + answer[0]);
            saveWord(answer);
            return;
        }
    }

    private Address findAddressByString(String str) {
        List<Address> addresses = channel.getView().getMembers().stream().filter(address -> {
            return address.toString().equals(str);
        }).collect(Collectors.toList());
        return addresses.get(0);
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

    private void saveWord(String[] answer) {
        roundHistory.add(new Answer(answer[0], answer[1]));
        Address address = findAddressByString(answer[0]);
        if (address != null) {
            Message msg = new ObjectMessage(address, MSG_gotItRight);
            try {
                channel.send(msg);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    private boolean isLeader() {
        if (leader == null) {
            leader = channel.getAddress();
            score.updateLeader(leader);
        }
        Address currentAddress = channel.getAddress();
        return leader.equals(currentAddress);
    }

    private void start() throws Exception {
        channel = new JChannel("/home/leonardo/Documentos/devTools/jgroups/udp.xml");
        channel.setReceiver(this);
        channel.connect("ChatCluster");

        score.addPlayer(channel.getAddress());
        Thread thread = new Thread() {
            public void run() {
                try {
                    dispatchDrawApp();
                } catch (Exception e) {
                    System.out.println("Error: did not start draw app");
                    e.printStackTrace();
                }
            }
        };
        thread.start();
        eventLoop();
        thread.join();
        channel.close();
    }

    private void dispatchDrawApp() throws Exception {
        JChannel drawChannel = new JChannel("/home/leonardo/Documentos/devTools/jgroups/udp.xml");
        this.draw = new MyDraw(drawChannel, isLeader());
        this.draw.setClusterName("DrawCluster");
        this.draw.go();
        drawChannel.close();
    }

    private void eventLoop() {
        BufferedReader in = new BufferedReader(new InputStreamReader(System.in));

        while (true) {
            try {
                System.out.print("> ");
                System.out.flush();
                String line = in.readLine().toLowerCase();
                if (line.startsWith("quit") || line.startsWith("exit")) {
                    break;
                }
                if (line.equals("/start") && wordToDraw.isEmpty() && isLeader()) {
                    startRound();
                } else {
                    line = channel.getAddressAsString() + ":" + line;
                    Message msg = new ObjectMessage(null, line);
                    channel.send(msg);
                }
            } catch (Exception e) {
            }
        }
    }

    private void startRound() throws Exception {
        channel.getView().getMembers().forEach(address -> {
            score.addPlayer(address);
        });
        score.updateLeader(channel.getAddress());

        Message msg = new ObjectMessage(null, MSG_warningStartRound);
        channel.send(msg);


        wordToDraw = sortNextWord();
        System.out.println(MSG_wordToDraw + wordToDraw);
        Thread.sleep(1000);

        draw.clearPanel();

        Date ts = new Date();
        msg = new ObjectMessage(null, MSG_roundStarted + ts.getTime());
        channel.send(msg);
        Thread.sleep(1000 * 20);
        System.out.println(MSG_roundFinished);
        ts = new Date();
        msg = new ObjectMessage(null, MSG_roundFinished + ts.getTime());
        channel.send(msg);
        finishRound();
    }

    private void finishRound() {
        List<Answer> pointedAnswers = new ArrayList<>();
        int pointCounter = 10;
        int leaderPoints = 0;
        while (!this.roundHistory.isEmpty()) {
            Answer answer = this.roundHistory.remove();
            answer.setPoints(pointCounter);
            pointedAnswers.add(answer);
            System.out.println("Answer: " + answer.toString());
            if (pointCounter > 1) {
                pointCounter--;
            }
            leaderPoints++;
            score.updateScore(answer.address, answer.getPoints());
        }

        leaderPoints = leaderPoints * 2;
        score.updateScore(channel.getAddress().toString(), leaderPoints);


        Message msg = new ObjectMessage(null, MSG_updateScore+score.toSerializable());
        try {
            channel.send(msg);
        } catch (Exception e) {
            e.printStackTrace();
            System.out.println("Error in communicating the score after round finished");
        }

        draw.clearPanel();
        wordToDraw = "";
        roundHistory = new LinkedList<Answer>();
        Address address = score.electNewLeader();
        msg = new ObjectMessage(null, MSG_newLeader + address.toString());
        try {
            channel.send(msg);
        } catch (Exception e) {
            e.printStackTrace();
            System.out.println("Error in communicating the new leader");
        }
    }

    private String sortNextWord() {
        // TODO implement a decent word sorter
/*
        ObjectMapper mapper = new ObjectMapper();

        //JSON file to Java object
        Staff obj = mapper.readValue(new File("c:\\test\\staff.json"), Word.class);

        //JSON URL to Java object
        Staff obj = mapper.readValue(new URL("http://some-domains/api/staff.json"), Staff.class);

        //JSON string to Java Object
        Staff obj = mapper.readValue("{'name' : 'mkyong'}", Staff.class);

 */
        return "hardcoded";
    }

    public static void main(String[] args) throws Exception {
        new SimpleChat().start();
    }
}